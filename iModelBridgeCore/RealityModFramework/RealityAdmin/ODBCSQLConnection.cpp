/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
#include <algorithm>  
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
// @bsimethod                                   Alain.Robert           	    11/2016
//-------------------------------------------------------------------------------------
SQLRETURN ServerConnection::FetchIdentityFromStringVal(SQLINTEGER &id, const char* tableName, const char* columnName, const char* requiredValue, SQLLEN &len)
    {
    SQLRETURN retCode;
    CHAR ident[256];
    sprintf(ident, "SELECT ID FROM [%s].[dbo].[%s] WHERE [%s] = '%s'", m_dbName.c_str(), tableName, columnName, requiredValue);
    ExecuteSQL(ident);
    SQLBindCol(hStmt, 1, SQL_INTEGER, &id, 2, &len);
    TryODBC(hStmt, SQL_HANDLE_STMT, retCode = SQLFetch(hStmt));
    ReleaseStmt();
    return retCode;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert           	    11/2016
//-------------------------------------------------------------------------------------
SQLRETURN ServerConnection::FetchIdentityFromDualStringVal(SQLINTEGER &id,  const char* tableName1, const char* columnName1, const char* requiredValue1, const char* columnName2, const char* requiredValue2, SQLLEN &len)
    {
    SQLRETURN retCode;
    CHAR ident[256];
    sprintf(ident, "SELECT ID FROM [%s].[dbo].[%s] WHERE [%s] = '%s' AND [%s] = '%s'", m_dbName.c_str(), tableName1, columnName1, requiredValue1, columnName2, requiredValue2);
    ExecuteSQL(ident);
    SQLBindCol(hStmt, 1, SQL_INTEGER, &id, 2, &len);
    TryODBC(hStmt, SQL_HANDLE_STMT, retCode = SQLFetch(hStmt));
    ReleaseStmt();
    return retCode;
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
    sprintf(preQuery, "SELECT * FROM [%s].[dbo].[Servers] WHERE [URL] = '%s'", m_dbName.c_str(), server.GetUrl().c_str());
    CHAR serverStatement[1024];
    if (HasEntries(preQuery))
        {
        sprintf(serverStatement, "UPDATE [%s].[dbo].[Servers] SET [LastCheck] = ?, [LastTimeOnline] = ? WHERE [URL] = '%s'",
            m_dbName.c_str(),
            server.GetUrl().c_str());
        }
    else
        {
        CHAR tempStatement[1024];
        CHAR tempStatement2[128];
        sprintf(tempStatement, "INSERT INTO [%s].[dbo].[Servers] ([CommunicationProtocol], [Name], [URL], [Online], [LastCheck], [LastTimeOnline]%s) VALUES ('%s', '%s', '%s', %d, ?, ?%s)",
            m_dbName.c_str(),
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
    sprintf(tableName, "[%s].[dbo].[Servers]", m_dbName.c_str());
    FetchIdentityFromStringVal(id, "Servers", "URL", server.GetUrl().c_str(), len);

    return id;
    }



//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
ODBCConnectionStatus ServerConnection::SaveMetadata(SpatialEntityMetadataCR metadata, SQLINTEGER& metadataId)
    {
    RETCODE retCode;
    
    if (!metadata.IsEmpty())
        {
        Utf8String metadataQuery;
        metadataQuery = "INSERT INTO [";
        metadataQuery += m_dbName;
        metadataQuery += "].[dbo].[Metadatas] (";

        Utf8String separator = "";
        Utf8String commaSeparator = ", ";

        if (metadata.GetDescription().size() != 0)
            {
            metadataQuery += separator + "[Description]";
            separator = commaSeparator;
            }

        if (metadata.GetProvenance().size() != 0)
            {
            metadataQuery += separator + "[Provenance]";
            separator = commaSeparator;
            }

        if (metadata.GetMetadataUrl().size() != 0)
            {
            metadataQuery += separator + "[MetadataURL]";
            separator = commaSeparator;
            }

        if (metadata.GetLegal().size() != 0)
            {
            metadataQuery += separator + "[Legal]";
            separator = commaSeparator;
            }


        if (metadata.GetTermsOfUse().size() != 0)
            {
            metadataQuery += separator + "[TermsOfUse]";
            separator = commaSeparator;
            }

        if (metadata.GetContactInfo().size() != 0)
            {
            metadataQuery += separator + "[ContactInformation]";
            separator = commaSeparator;
            }

        //if (metadata.GetFormat().size() != 0)
        //    {
        //    metadataQuery += separator + "[RawMetadataFormat]";
        //    separator = commaSeparator;
        //    }

        metadataQuery += ") VALUES (";

        separator = "";


        if (metadata.GetDescription().size() != 0)
            {
            metadataQuery += separator + "'" + metadata.GetDescription() + "'";
            separator = commaSeparator;
            }

        if (metadata.GetProvenance().size() != 0)
            {
            metadataQuery += separator + "'" + metadata.GetProvenance() + "'";
            separator = commaSeparator;
            }

        if (metadata.GetMetadataUrl().size() != 0)
            {
            metadataQuery += separator + "'" + metadata.GetMetadataUrl() + "'";
            separator = commaSeparator;
            }

        if (metadata.GetLegal().size() != 0)
            {
            metadataQuery += separator + "'" + metadata.GetLegal() + "'";
            separator = commaSeparator;
            }

        if (metadata.GetTermsOfUse().size() != 0)
            {
            metadataQuery += separator + "'" + metadata.GetTermsOfUse() + "'";
            separator = commaSeparator;
            }

        if (metadata.GetContactInfo().size() != 0)
            {
            metadataQuery += separator + "'" + metadata.GetContactInfo() + "'";
            separator = commaSeparator;
            }

        metadataQuery += ")";

        CHAR finalMetadataQuery[512];
        strcpy_s(finalMetadataQuery, 512, metadataQuery.c_str());
        retCode = ExecuteSQL(finalMetadataQuery);
        ReleaseStmt();
        }

    SQLLEN len;
    CHAR tableName[128];
    sprintf(tableName, "[%s].[dbo].[Metadatas]", m_dbName.c_str());
    FetchTableIdentity(metadataId, tableName, len);

    return ODBCConnectionStatus::Success;
    }

#if (0)
//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
ODBCConnectionStatus ServerConnection::SaveThumbnail(SpatialEntityThumbnailCR thumbnail, SQLINTEGER& thumbnailId)
    {
    DateTimeCR date = DateTime::GetCurrentTimeUtc();
    bool thumbnailPresent = false;
    RETCODE retCode;


    if (!thumbnail.IsEmpty())
        {
        Utf8String thumbnailQuery;
        thumbnailQuery = "INSERT INTO [";
        thumbnailQuery += m_dbName;
        thumbnailQuery += "].[dbo].[Thumbnails] (";

        Utf8String separator = "";
        Utf8String commaSeparator = ", ";

        CHAR* finalThumbnailQuery;
        const bvector<Byte>& thumbnailBytes = thumbnail.GetData();
        size_t size = thumbnailBytes.size();
        if (0 != size)
            thumbnailPresent = true;


        if (thumbnail.GetProvenance().size() != 0)
            {
            thumbnailQuery += separator + "[ThumbnailProvenance]";
            separator = commaSeparator;
            }

        if (thumbnail.GetFormat().size() != 0)
            {
            thumbnailQuery += separator + "[ThumbnailFormat]";
            separator = commaSeparator;
            }

        // We update the date stamp whatever the case.
        thumbnailQuery += separator + "[ThumbnailStamp]";
        separator = commaSeparator;


        if (thumbnail.GetProvenance().size() != 0)
            {
            thumbnailQuery += separator + "[ThumbnailGenerationDetails]";
            }
        
        if(thumbnail.GetThumbnailUrl().length() > 0)
            {
            thumbnailQuery += separator + "[ThumbnailUrl]";
            }

        if (thumbnailPresent)
            {
            thumbnailQuery += separator + "[ThumbnailData]";
            }

        thumbnailQuery += ") VALUES (";

        separator = "";

        if (thumbnail.GetProvenance().size() != 0)
            {
            thumbnailQuery += separator + "'" + thumbnail.GetProvenance() + "'";
            separator = commaSeparator;
            }

        if (thumbnail.GetFormat().size() != 0)
            {
            thumbnailQuery += separator + "'" + thumbnail.GetFormat() + "'";
            separator = commaSeparator;
            }

        thumbnailQuery += separator + "'" + date.ToUtf8String() + "'";
        separator = commaSeparator;


        if (thumbnail.GetProvenance().size() != 0)
            {
            thumbnailQuery += separator + "'" + thumbnail.GetProvenance() + "'";
            }

        if (thumbnail.GetThumbnailUrl().size() != 0)
            {
            thumbnailQuery += separator + "'" + thumbnail.GetThumbnailUrl() + "'";
            }

        if (thumbnailPresent)
            {
            thumbnailQuery += separator + "?";
            }

        thumbnailQuery += ")";

        unsigned char* dataArray;

        if (thumbnailPresent)
            {
            finalThumbnailQuery = new CHAR[1048576];
            strcpy_s(finalThumbnailQuery, 1024, thumbnailQuery.c_str());

            dataArray = new unsigned char[size];
            for (int i = 0; i < size; ++i)
                dataArray[i] = thumbnailBytes[i];

            SQLPrepare(hStmt, (SQLCHAR*)finalThumbnailQuery, SQL_NTS);

            SQLLEN sqlLength = size;

            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY, size, 0, dataArray, size, &sqlLength);

            retCode = ExecuteSQL(hStmt);

            ReleaseStmt();
            delete[] dataArray;


            }
        else
            {
            finalThumbnailQuery = new CHAR[1024];
            strcpy_s(finalThumbnailQuery, 1024, thumbnailQuery.c_str());

            retCode = ExecuteSQL(finalThumbnailQuery);
            ReleaseStmt();

            }   


        delete [] finalThumbnailQuery;

        SQLLEN len;
        CHAR tableName[128];

        sprintf(tableName, "[%s].[dbo].[Thumbnails]", m_dbName.c_str());
        FetchTableIdentity(thumbnailId, tableName, len);
        }

    return ODBCConnectionStatus::Success;
    }

#endif

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   11/2016
//-------------------------------------------------------------------------------------
ODBCConnectionStatus ServerConnection::SaveDataSource(SpatialEntityDataSourceCR source, SQLINTEGER dataId, SQLINTEGER serverId, SQLINTEGER& dataSourceId)
    {
    RETCODE retCode;

    //if (!source.IsEmpty())
        {
        SQLINTEGER dataSize = (int)source.GetSize();

        Utf8String sourceQuery;
        sourceQuery = "INSERT INTO [";
        sourceQuery += m_dbName;
        sourceQuery += "].[dbo].[SpatialDataSources] ([MainURL]";

        if (source.GetCompoundType().size() != 0)
            sourceQuery += ", [CompoundType]";

        if (dataSize != 0)
            sourceQuery += ", [FileSize]";

        if (source.GetGeoCS().size() != 0)
            sourceQuery += ", [CoordinateSystem]";

        // Data type is mandatory
        sourceQuery += ", [DataSourceType]";

        if (source.GetLocationInCompound().size() != 0)
            sourceQuery += ", [LocationInCompound]";

        if (source.GetNoDataValue().size() != 0)
            sourceQuery += ", [NoDataValue]";

        if (serverId != 0)
            sourceQuery += ", [Server_Id]";

        if (dataId != 0)
            sourceQuery += ", [SpatialEntity_Id]";

        sourceQuery += ") VALUES ('" + source.GetUri().ToString() + "'";

        if (source.GetCompoundType().size() != 0)
            sourceQuery += ", '" + source.GetCompoundType() + "'";

        if (dataSize != 0)
            {
            char dataSizeStr[25];
            sprintf(dataSizeStr, "%d", dataSize);
            sourceQuery += ", ";
            sourceQuery += dataSizeStr;
            }

        if (source.GetGeoCS().size() != 0)
            sourceQuery += ", '" + source.GetGeoCS() + "'";

        // Data type is mandatroy
        sourceQuery += ", '" + source.GetDataType() + "'";

        if (source.GetLocationInCompound().size() != 0)
            sourceQuery += ", '" + source.GetLocationInCompound() + "'";

        if (source.GetNoDataValue().size() != 0)
            sourceQuery += ", '" + source.GetNoDataValue() + "'";

        if (serverId != 0)
            {
            char serverIdStr[25];
            sprintf(serverIdStr, "%d", serverId);
            sourceQuery += ", ";
            sourceQuery += serverIdStr;
            }

        if (dataId != 0)
            {
            char spatialEntityIdStr[25];
            sprintf(spatialEntityIdStr, "%d", dataId);
            sourceQuery += ", ";
            sourceQuery += spatialEntityIdStr;
            }

        sourceQuery += ")";

        CHAR finalSourceQuery[1024];
        strcpy_s(finalSourceQuery, 1024, sourceQuery.c_str());

        retCode = ExecuteSQL(finalSourceQuery);
        ReleaseStmt();

        SQLLEN len;
        CHAR tableName[128];

        sprintf(tableName, "[%s].[dbo].[SpatialDataSources]", m_dbName.c_str());
//        FetchIdentityFromStringVal(dataSourceId, "[SpatialDataSources]", , )
        FetchTableIdentity(dataSourceId, tableName, len);

        ReleaseStmt();


        // Paragraph that deals with multiband sources TODO
        /*if(source.GetIsMultiband())
            {
            Utf8String blueUrl;
            Utf8String greenUrl;
            Utf8String redUrl;
            Utf8String panUrl;
            source.GetMultibandUrls(redUrl, greenUrl, blueUrl, panUrl);


            CHAR multiBQuery[1024];


            sprintf(multiBQuery, "INSERT INTO [%s].[dbo].[MultibandSources] ([Id], [RedBandURL], [RedBandFileSize], [GreenBandURL], [GreenBandFileSize], [BlueBandURL], [BlueBandFileSize], [PanchromaticBandURL], [PanchromaticBandFileSize]) VALUES ( %d, '%s', %lld, '%s', %lld, '%s', %lld, '%s', %lld)",
                    m_dbName.c_str(),
                    dataSourceId,
                    redUrl.c_str(),
                    source.GetRedBandSize(),
                    greenUrl.c_str(),
                    source.GetGreenBandSize(),
                    blueUrl.c_str(),
                    source.GetBlueBandSize(),
                    panUrl.c_str(),
                    source.GetPanchromaticBandSize());
  

            ExecuteSQL(multiBQuery);
            ReleaseStmt();
            }*/
        }

    return ODBCConnectionStatus::Success;

    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
ODBCConnectionStatus ServerConnection::InsertBareSpatialEntity(SpatialEntityCR data, SQLINTEGER metadataId, SQLINTEGER thumbnailId, SQLINTEGER& entityId)
    {
    ODBCConnectionStatus result = ODBCConnectionStatus::Success;
    RETCODE retCode;
    SQLLEN len;

    //&&AR TODO ... should make sure entry does not exist.

    DRange2dCR Fpt = data.GetFootprintExtent();
    double xMin = std::min(Fpt.low.x, Fpt.high.x);
    double xMax = std::max(Fpt.low.x, Fpt.high.x);
    double yMin = std::min(Fpt.low.y, Fpt.high.y);
    double yMax = std::max(Fpt.low.y, Fpt.high.y);

    DateTimeCR date = data.GetDate();

    Utf8String entityBaseQuery;
    entityBaseQuery = "INSERT INTO [";
    entityBaseQuery += m_dbName;
    entityBaseQuery += "].[dbo].[SpatialEntities] ([Name]";

    if (data.GetProvider().size() != 0)
        entityBaseQuery += ", [DataProvider]";

    if (data.GetProviderName().size() != 0)
        entityBaseQuery += ", [DataProviderName]";

    // Dataset is mandatory (but can be an empty string)
    entityBaseQuery += ", [Dataset]";

    // Footprint is mandatory
    entityBaseQuery += ", [Footprint]";

    // Approximate footprint is mandatory
    entityBaseQuery += ", [ApproximateFootprint]";

    // Bounding box is mandatory
    entityBaseQuery += ", [MinX], [MinY], [MaxX], [MaxY]";

    if(data.GetOcclusion() >= 0.0f)
        entityBaseQuery += ", [Occlusion]";

    if (date.IsValid())
        entityBaseQuery += ", [Date]";

    if (metadataId != 0)
        entityBaseQuery += ", [Metadata_Id]";

    if (data.GetDataType().size() != 0)
        entityBaseQuery += ", [DataSourceTypesAvailable]";

    if (data.GetResolution().size() != 0)
        entityBaseQuery += ", [ResolutionInMeters]";

    // Classification is mandatory
    entityBaseQuery += ", [Classification]";

    if (data.GetThumbnailURL().size() != 0)
        entityBaseQuery += ", [ThumbnailURL]";

    if (data.GetThumbnailLoginKey().size() != 0)
        entityBaseQuery += ", [ThumbnailLoginKey]";

    entityBaseQuery += ") VALUES ('" + data.GetName() + "'";

    if (data.GetProvider().size() != 0)
        entityBaseQuery += ", '" + data.GetProvider() + "'";

    if (data.GetProviderName().size() != 0)
        entityBaseQuery += ", '" + data.GetProviderName() + "'";

    // Dataset is mandatory (but can be an empty string)
    entityBaseQuery += ", '" + data.GetDataset() + "'";

    // Footprint placeholder
    entityBaseQuery += ", geometry::STPolyFromText(?, 4326)";

    if (data.HasApproximateFootprint())
        entityBaseQuery += ", 1";
    else
        entityBaseQuery += ", 0";

    char boundingBoxString[128];
    sprintf(boundingBoxString, ", %f, %f, %f, %f", xMin, yMin, xMax, yMax);
    entityBaseQuery += boundingBoxString;

    if(data.GetOcclusion() >= 0.0f)
        {
        char occlusionStr[25];
        sprintf(occlusionStr, "%f", data.GetOcclusion());
        entityBaseQuery += ", ";
        entityBaseQuery += occlusionStr;
        }

    if (date.IsValid())
        entityBaseQuery += ", ?";

    if (metadataId != 0)
        {
        char metadataIdStr[25];
        sprintf(metadataIdStr, "%d", metadataId);
        entityBaseQuery += ", ";
        entityBaseQuery += metadataIdStr;
        }

    if (data.GetDataType().size() != 0)
        entityBaseQuery += ", '" + data.GetDataType() + "'";

    if (data.GetResolution().size() != 0)
        entityBaseQuery += ", '" + data.GetResolution() + "'";

    // Classification is mandatory
    entityBaseQuery += ", '" + data.GetClassificationTag() + "'";

    if (data.GetThumbnailURL().size() != 0)
        entityBaseQuery += ", '" + data.GetThumbnailURL() + "'";

    if (data.GetThumbnailLoginKey().size() != 0)
        entityBaseQuery += ", '" + data.GetThumbnailLoginKey() + "'";

    entityBaseQuery += ")";

    CHAR finalentityBaseQuery[2048];
    strcpy_s(finalentityBaseQuery, 2048, entityBaseQuery.c_str());

    SQLPrepare(hStmt, (SQLCHAR*)finalentityBaseQuery, SQL_NTS);


    char* polygon = new char[2048];
    sprintf(polygon, "POLYGON((%f %f, %f %f, %f %f, %f %f, %f %f))", xMax, yMax, xMax, yMin, xMin, yMin, xMin, yMax, xMax, yMax);
    retCode = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_LONGVARCHAR, strlen(polygon), 0, (SQLPOINTER)polygon, strlen(polygon), NULL);

        
    CHAR baseDate[32];
    sprintf(baseDate, "%d-%d-%d", date.GetYear(), date.GetMonth(), date.GetDay());
    TryODBC(hStmt, SQL_HANDLE_STMT, SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, strlen(baseDate), 0, (SQLPOINTER)baseDate, strlen(baseDate), NULL));

    ExecuteSQL(hStmt);
    ReleaseStmt();
    delete[] polygon;


    if (data.GetDataset().size() != 0)
        retCode = FetchIdentityFromDualStringVal(entityId, "SpatialEntities", "Name", data.GetName().c_str(), "Dataset", data.GetDataset().c_str(), len);
    else
        retCode = FetchIdentityFromStringVal(entityId, "SpatialEntities", "Name", data.GetName().c_str(), len);


#if (0)
    CHAR entityQuery[256];
    if(data.GetOcclusion() >= 0.0f)
        sprintf(entityQuery, "INSERT INTO [%s].[dbo].[SpatialEntities] ([Id], [Occlusion]) VALUES (%d, %f)",
            m_dbName.c_str(),
            entityId,
            data.GetOcclusion());
    else
        sprintf(entityQuery, "INSERT INTO [%s].[dbo].[SpatialEntities] ([Id]) VALUES (%d)",
            m_dbName.c_str(),
            entityId);
    ExecuteSQL(entityQuery);
    ReleaseStmt();
#endif

    return result;

    
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
ODBCConnectionStatus ServerConnection::SaveSpatialEntity(SpatialEntityCR data, bool dualMode)
    {
    // CHAR preQuery[512];
    ODBCConnectionStatus result;
    SQLINTEGER metadataId = 0;
    RETCODE retCode;
    SQLLEN len;

    SQLINTEGER thumbnailId = 0;

#if (0)
    sprintf(preQuery, "SELECT * FROM [%s].[dbo].[SpatialDataSources] WHERE [MainURL] = '%s'", m_dbName.c_str(), data.GetDataSource().GetUrl().c_str());
    if (HasEntries(preQuery))
        return ODBCConnectionStatus::RecordAlreadyExistsError;
#endif
    
#if (0)
    if (ODBCConnectionStatus::Success != (result = ServerConnection::SaveThumbnail(data.GetThumbnail(), thumbnailId)))
        return result;
#endif


    CHAR existingEntityBaseQuery[512];
    sprintf(existingEntityBaseQuery, "SELECT [Id] FROM [%s].[dbo].[SpatialEntities] WHERE [Name] = '%s' AND [Dataset] = '%s'", m_dbName.c_str(), data.GetName().c_str(), data.GetDataset().c_str());
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

    if (hasExisting)
        {
        //&&AR Verify consistency saving a spatial entity that exists may indicate we are adding a new data source to the same entry
        return ODBCConnectionStatus::RecordAlreadyExistsError;
        }
    else
        {
        if (data.GetMetadataCP() != NULL)
            {
            if (ODBCConnectionStatus::Success != (result = ServerConnection::SaveMetadata(*(data.GetMetadataCP()), metadataId)))
                return result;

            if (ODBCConnectionStatus::Success != (result = ServerConnection::InsertBareSpatialEntity(data, metadataId, thumbnailId, entityId)))
                return result;
            }
        }

    SQLINTEGER dataSourceId;

    // &&AR Remove the server id from the structure
    for (size_t index = 0 ; index < data.GetDataSourceCount() ; index++)
        {
        if (ODBCConnectionStatus::Success != (result = ServerConnection::SaveDataSource(data.GetDataSource(index), entityId, data.GetDataSource(index).GetServerId(), dataSourceId)))
            return result;
        }
        
    return ODBCConnectionStatus::Success;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert                   11/2016
//-------------------------------------------------------------------------------------
ODBCConnectionStatus ServerConnection::UpdateBareSpatialEntity(SpatialEntityCR data, SQLINTEGER spatialEntityId)
    {
    DRange2dCR Fpt = data.GetFootprintExtent();
    double xMin = std::min(Fpt.low.x, Fpt.high.x);
    double xMax = std::max(Fpt.low.x, Fpt.high.x);
    double yMin = std::min(Fpt.low.y, Fpt.high.y);
    double yMax = std::max(Fpt.low.y, Fpt.high.y);

    DateTimeCR date = data.GetDate();
    CHAR entityBaseQuery[4096];
    sprintf(entityBaseQuery, "UPDATE [%s].[dbo].[SpatialEntities] SET [Name] = '%s'",         
        m_dbName.c_str(),
        data.GetName().c_str());

    if (data.GetResolution().size() != 0)
        sprintf(entityBaseQuery, "%s, [ResolutionInMeters] = '%s'", entityBaseQuery, data.GetResolution().c_str());

    if (data.GetProvider().size() != 0)
        sprintf(entityBaseQuery, "%s, [DataProvider] = '%s'", entityBaseQuery, data.GetProvider().c_str());

    if (data.GetProviderName().size() != 0)
        sprintf(entityBaseQuery, "%s, [DataProviderName] = '%s'", entityBaseQuery, data.GetProviderName().c_str());

    if (data.GetDataset().size() != 0)
        sprintf(entityBaseQuery, "%s, [Dataset] = '%s'", entityBaseQuery, data.GetDataset().c_str());

    if (data.GetClassification() != SpatialEntity::Classification::UNDEFINED_CLASSIF)
        sprintf(entityBaseQuery, "%s, [Classification] = '%s'", entityBaseQuery, data.GetClassificationTag().c_str());

    if (xMin < xMax && yMin < yMax)
        sprintf(entityBaseQuery, "%s, [Footprint] = geometry::STPolyFromText(?, 4326), [MinX] = %f, [MinY] = %f, [MaxX] = %f, [MaxY] = %f", entityBaseQuery, xMin, yMin, xMax, yMax);

    if (data.HasApproximateFootprint())
        sprintf(entityBaseQuery, "%s, [ApproximateFootprint] = %d", entityBaseQuery, 1);
    else
        sprintf(entityBaseQuery, "%s, [ApproximateFootprint] = %d", entityBaseQuery, 0);

    if(data.GetOcclusion() >= 0.0f)
        sprintf(entityBaseQuery, "%s, [Occlusion] = %f", entityBaseQuery, data.GetOcclusion());

    if (date.IsValid())
        sprintf(entityBaseQuery, "%s, [Date] = '%d-%d-%d'", entityBaseQuery, date.GetYear(), date.GetMonth(), date.GetDay());

    if (data.GetDataType().size() != 0)
        sprintf(entityBaseQuery, "%s, [DataSourceTypesAvailable] = '%s'", entityBaseQuery, data.GetDataType().c_str());

    sprintf(entityBaseQuery, "%s WHERE [Id] = %d", entityBaseQuery, spatialEntityId);

    SQLPrepare(hStmt, (SQLCHAR*)entityBaseQuery, SQL_NTS);

    char* polygon = new char[2048];
    if (xMin < xMax && yMin < yMax)
        {
        sprintf(polygon, "POLYGON((%f %f, %f %f, %f %f, %f %f, %f %f))", xMax, yMax, xMax, yMin, xMin, yMin, xMin, yMax, xMax, yMax);
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_LONGVARCHAR, strlen(polygon), 0, (SQLPOINTER)polygon, strlen(polygon), NULL);
        }

    ExecuteSQL(hStmt);
    ReleaseStmt();
    delete[] polygon;

    return ODBCConnectionStatus::Success;
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert            	    8/2016
//-------------------------------------------------------------------------------------
ODBCConnectionStatus ServerConnection::UpdateDataSource(SpatialEntityDataSourceCR dataSource, SQLINTEGER dataSourceId)
    {
    // ODBCConnectionStatus result = ODBCConnectionStatus::Success;

    if ((dataSource.GetCompoundType().size() != 0) || (dataSource.GetSize() != 0) || (dataSource.GetDataType().size() != 0) || (dataSource.GetGeoCS().size() != 0))
        {
        // At least one property is not empty ...
        Utf8String dataSourceQuery;
        dataSourceQuery = "UPDATE [";
        dataSourceQuery += m_dbName;
        dataSourceQuery += "].[dbo].[SpatialDataSources] SET ";

        Utf8String separator = "";
        Utf8String commaSeparator = ", ";

        if (dataSource.GetCompoundType().size() != 0)
            {
            dataSourceQuery += separator + "[CompoundType] = ";
            dataSourceQuery += "'" + dataSource.GetCompoundType() + "'";
            separator = commaSeparator;
            }

        if (dataSource.GetSize() != 0)
            {
            dataSourceQuery += separator + "[FileSize] = ";
            char sizeStr[25];
            sprintf(sizeStr, "%lld", dataSource.GetSize());
            dataSourceQuery += sizeStr;
            separator = commaSeparator;
            }

        if (dataSource.GetDataType().size() != 0)
            {
            dataSourceQuery += separator + "[DataSourceType] = ";
            dataSourceQuery += "'" + dataSource.GetDataType() + "'";
            separator = commaSeparator;
            }

        if (dataSource.GetGeoCS().size() != 0)
            {
            dataSourceQuery += separator + "[CoordinateSystem] = ";
            dataSourceQuery += "'" + dataSource.GetGeoCS() + "'";
            separator = commaSeparator;
            }

        dataSourceQuery += " WHERE [ID] = ";
        char dataSourceString[25];
        sprintf(dataSourceString, "%d", dataSourceId);
        dataSourceQuery += dataSourceString;

        CHAR finalDataSourceQuery[1024];
        strcpy_s(finalDataSourceQuery, 1024, dataSourceQuery.c_str());

        ExecuteSQL(finalDataSourceQuery);
        ReleaseStmt();

        }

    




    // Check if the datasource is a multiband TODO
    /*if (dataSource.GetIsMultiband())
        {
        // Check if any value needs update
        Utf8String blueUrl;
        Utf8String greenUrl;
        Utf8String redUrl;
        Utf8String panUrl;
        dataSource.GetMultibandUrls(redUrl, greenUrl, blueUrl, panUrl);

        Utf8String multiBandQuery;

        multiBandQuery = "UPDATE [";
        multiBandQuery += m_dbName;
        multiBandQuery += "].[dbo].[MultibandSources] SET [RedBandURL] = '" + redUrl + "' , [GreenBandURL] = '" + greenUrl + "' , [BlueBandURL] = '" + blueUrl + "' , [PanchromaticBandURL] = '" + panUrl + "'";

        char multiBandString[25];

        if (dataSource.GetRedBandSize() != 0 || dataSource.GetGreenBandSize() != 0 || dataSource.GetBlueBandSize() != 0 || dataSource.GetPanchromaticBandSize() != 0)
            {
            multiBandQuery += ",  [RedBandFileSize] = ";
            sprintf(multiBandString, "%lld", dataSource.GetRedBandSize());
            multiBandQuery += multiBandString;

            multiBandQuery += ",  [GreenBandFileSize] = ";
            sprintf(multiBandString, "%lld", dataSource.GetRedBandSize());
            multiBandQuery += multiBandString;

            multiBandQuery += ",  [BlueBandFileSize] = ";
            sprintf(multiBandString, "%lld", dataSource.GetRedBandSize());
            multiBandQuery += multiBandString;

            multiBandQuery += ",  [PanchromaticBandFileSize] = ";
            sprintf(multiBandString, "%lld", dataSource.GetPanchromaticBandSize());
            multiBandQuery += multiBandString;
            }

        multiBandQuery += " WHERE [ID] = ";
        sprintf(multiBandString, "%d", dataSourceId);
        multiBandQuery += multiBandString;

        CHAR finalMultiBandQuery[1024];
        strcpy_s(finalMultiBandQuery, 1024, multiBandQuery.c_str());

        ExecuteSQL(finalMultiBandQuery);
        ReleaseStmt();
        }*/


    return ODBCConnectionStatus::Success;
    
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert            	    8/2016
//-------------------------------------------------------------------------------------
ODBCConnectionStatus ServerConnection::UpdateMetadata(SpatialEntityMetadataCR metadata, SQLINTEGER metadataId)
    {
    // ODBCConnectionStatus result = ODBCConnectionStatus::Success;

    if (!metadata.IsEmpty())
        {
        // At least one property is not empty ...
        Utf8String metadataQuery;
        metadataQuery = "UPDATE [";
        metadataQuery += m_dbName;
        metadataQuery += "].[dbo].[Metadatas] SET ";

        Utf8String separator = "";
        Utf8String commaSeparator = ", ";

        if (metadata.GetProvenance().size() != 0)
            {
            metadataQuery += separator + "[Provenance] = ";
            metadataQuery += "'" + metadata.GetProvenance() + "'";
            separator = commaSeparator;
            }

        if (metadata.GetDescription().size() != 0)
            {
            metadataQuery += separator + "[Description] = ";
            metadataQuery += "'" + metadata.GetDescription() + "'";
            separator = commaSeparator;
            }        

        if (metadata.GetContactInfo().size() != 0)
            {
            metadataQuery += separator + "[ContactInformation] = ";
            metadataQuery += "'" + metadata.GetContactInfo() + "'";
            separator = commaSeparator;
            }    

        if (metadata.GetLegal().size() != 0)
            {
            metadataQuery += separator + "[Legal] = ";
            metadataQuery += "'" + metadata.GetLegal() + "'";
            separator = commaSeparator;
            }    

        if (metadata.GetTermsOfUse().size() != 0)
            {
            metadataQuery += separator + "[TermsOfUse] = ";
            metadataQuery += "'" + metadata.GetTermsOfUse() + "'";
            separator = commaSeparator;
            }    

        if (metadata.GetFormat().size() != 0)
            {
            metadataQuery += separator + "[RawMetadataFormat] = ";
            metadataQuery += "'" + metadata.GetFormat() + "'";
            separator = commaSeparator;
            }    

        metadataQuery += " WHERE [ID] = ";
        char metadataString[25];
        sprintf(metadataString, "%d", metadataId);
        metadataQuery += metadataString;

        CHAR finalMetadataQuery[1024];
        strcpy_s(finalMetadataQuery, 1024, metadataQuery.c_str());

        ExecuteSQL(finalMetadataQuery);
        ReleaseStmt();

        }

    return ODBCConnectionStatus::Success;
    }

#if (0)
//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert            	    8/2016
//-------------------------------------------------------------------------------------
ODBCConnectionStatus ServerConnection::UpdateThumbnail(SpatialEntityThumbnailCR thumbnail, SQLINTEGER thumbnailId)
    {
    // ODBCConnectionStatus result = ODBCConnectionStatus::Success;
    char thumbnailString[25];
    SQLLEN len;

    if (!thumbnail.IsEmpty())
        {
        // At least one property is not empty ...
        Utf8String thumbnailQuery;
        thumbnailQuery = "UPDATE [";
        thumbnailQuery += m_dbName;
        thumbnailQuery += "].[dbo].[Thumbnails] SET ";

        Utf8String separator = "";
        Utf8String commaSeparator = ", ";

        if (thumbnail.GetProvenance().size() != 0)
            {
            thumbnailQuery += separator + "[ThumbnailProvenance] = ";
            thumbnailQuery += "'" + thumbnail.GetProvenance() + "'";
            separator = commaSeparator;
            }

        if (thumbnail.GetFormat().size() != 0)
            {
            thumbnailQuery += separator + "[ThumbnailFormat] = ";
            thumbnailQuery += "'" + thumbnail.GetFormat() + "'";
            separator = commaSeparator;
            }        

        if ((thumbnail.GetWidth() != 0) && (thumbnail.GetHeight() != 0))
            {
            thumbnailQuery += separator + "[ThumbnailWidth] = ";
            sprintf(thumbnailString, "%ld", thumbnail.GetWidth());
            thumbnailQuery += thumbnailString;
            thumbnailQuery += ", [ThumbnailHeight] = ";
            sprintf(thumbnailString, "%ld", thumbnail.GetHeight());
            thumbnailQuery += thumbnailString;
            separator = commaSeparator;
            }    

        if (thumbnail.GetStamp().IsValid())
            {
            thumbnailQuery += separator + "[ThumbnailStamp] = ?";
            separator = commaSeparator;
            }    


        if (thumbnail.GetGenerationDetails().size() != 0)
            {
            thumbnailQuery += separator + "[ThumbnailGenerationDetails] = ";
            thumbnailQuery += "'" + thumbnail.GetGenerationDetails() + "'";
            separator = commaSeparator;
            }    

        if (thumbnail.GetThumbnailUrl().size() != 0)
            {
            thumbnailQuery += separator + "[ThumbnailURL] = ";
            thumbnailQuery += "'" + thumbnail.GetThumbnailUrl() + "'";
            separator = commaSeparator;
            }    

        const bvector<Byte>& thumbnailBytes = thumbnail.GetData();
        size_t size = thumbnailBytes.size();

        if (size != 0)
            {
            thumbnailQuery += separator + "[ThumbnailData] = ?";
            separator = commaSeparator;
            }


        thumbnailQuery += " WHERE [ID] = ";
        sprintf(thumbnailString, "%d", thumbnailId);
        thumbnailQuery += thumbnailString;

        CHAR finalThumbnailQuery[1024];
        strcpy_s(finalThumbnailQuery, 1024, thumbnailQuery.c_str());

        SQLPrepare(hStmt, (SQLCHAR*)finalThumbnailQuery, SQL_NTS);
        SQLLEN dateSize = sizeof(SQL_TIMESTAMP_STRUCT);


        if (thumbnail.GetStamp().IsValid())
            {
            SQL_TIMESTAMP_STRUCT stampTime = PackageDateTime(thumbnail.GetStamp());
            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TYPE_TIMESTAMP, 23, 3, &stampTime, dateSize, 0);
            }

        unsigned char* dataArray;

        if (size != 0)
            {
            dataArray = new unsigned char[size];
            for (int i = 0; i < size; ++i)
                dataArray[i] = thumbnailBytes[i];
            SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY, size, 0, dataArray, size, &len);
            }
        ExecuteSQL(hStmt);
        ReleaseStmt();

        if (size != 0)
            delete[] dataArray;

        }
        return ODBCConnectionStatus::Success;
    }

#endif

//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert            	    11/2016
//-------------------------------------------------------------------------------------
ODBCConnectionStatus ServerConnection::GetMetadataIdFromSpatialEntity(SQLINTEGER entityId, SQLINTEGER& metadataId, bool& uniqueConsumer)
    {
    SQLLEN numberOfRecordReferingMetadata;
    SQLLEN len;
    SQLINTEGER statLen;
    //RETCODE retCode;



    CHAR metaIdQuery[256];
    sprintf(metaIdQuery, "SELECT [Metadata_Id] FROM [%s].[dbo].[SpatialEntities] WHERE [Id] = %d", m_dbName.c_str(), entityId);
    ExecuteSQL(metaIdQuery);
    SQLBindCol(hStmt, 1, SQL_INTEGER, &metadataId, 2, &len);

    TryODBC(hStmt, SQL_HANDLE_STMT, SQLFetch(hStmt));
    ReleaseStmt();

    sprintf(metaIdQuery, "SELECT [Id] FROM [%s].[dbo].[SpatialEntities] WHERE [Metadata_Id] = %d", m_dbName.c_str(), metadataId);
    ExecuteSQL(metaIdQuery);

    SQLGetStmtAttr(hStmt, SQL_ATTR_ROW_ARRAY_SIZE, &numberOfRecordReferingMetadata, 0, &statLen);

    uniqueConsumer = (numberOfRecordReferingMetadata == 1);
    ReleaseStmt();

    return ODBCConnectionStatus::Success;
    }

#if (0)
//-------------------------------------------------------------------------------------
// @bsimethod                                   Alain.Robert            	    11/2016
//-------------------------------------------------------------------------------------
ODBCConnectionStatus ServerConnection::GetThumbnailIdFromSpatialEntity(SQLINTEGER entityId, SQLINTEGER& thumbnailId, bool& uniqueConsumer)
    {
    SQLLEN numberOfRecordReferingThumbnail;
    SQLLEN len;
    // RETCODE retCode;
    SQLINTEGER statLen;




    CHAR thumbIdQuery[256];
    sprintf(thumbIdQuery, "SELECT [Thumbnail_Id] FROM [%s].[dbo].[SpatialEntities] WHERE [Id] = %d", m_dbName.c_str(), entityId);
    ExecuteSQL(thumbIdQuery);
    SQLBindCol(hStmt, 1, SQL_INTEGER, &thumbnailId, 2, &len);

    TryODBC(hStmt, SQL_HANDLE_STMT, SQLFetch(hStmt));
    ReleaseStmt();

    sprintf(thumbIdQuery, "SELECT [Id] FROM [%s].[dbo].[SpatialEntities] WHERE [Thumbnail_Id] = %d", m_dbName.c_str(), thumbnailId);
    ExecuteSQL(thumbIdQuery);

    SQLGetStmtAttr(hStmt, SQL_ATTR_ROW_ARRAY_SIZE, &numberOfRecordReferingThumbnail, 0, &statLen);

    uniqueConsumer = (numberOfRecordReferingThumbnail == 1);
    ReleaseStmt();
    return ODBCConnectionStatus::Success;

    }
#endif

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
ODBCConnectionStatus ServerConnection::Update(SpatialEntityCR data)
    {
    ODBCConnectionStatus result;
    RETCODE retCode;
    SQLINTEGER entityId = 0;
    SQLINTEGER dataSourceId = 0;
    SQLINTEGER metadataId = 0;
    SQLLEN len;


    // Check if spatial entity exists
    if (data.GetDataset().size() != 0)
        retCode = FetchIdentityFromDualStringVal(entityId, "SpatialEntities", "Name", data.GetName().c_str(), "Dataset", data.GetDataset().c_str(), len);
    else
        retCode = FetchIdentityFromStringVal(entityId, "SpatialEntities", "Name", data.GetName().c_str(), len);
 

    if (entityId > 0)
        {
        // Spatial entity already exist ...
        if (ODBCConnectionStatus::Success != (result = ServerConnection::UpdateBareSpatialEntity(data, entityId)))
            return result;
        }
    else
        {
        // Spatial entity does not exist
        return ODBCConnectionStatus::RecordDoesNotExistError;
        }

    // If metadata is provided ...
    if (data.GetMetadataCP() == NULL)
        {
        // Locate the metadata id in the spatial entity and the fact it is used by one or many sources.
        bool uniqueConsumer = true;
        result = GetMetadataIdFromSpatialEntity(entityId, metadataId, uniqueConsumer);

        if (ODBCConnectionStatus::Success == result && metadataId > 0 && uniqueConsumer)
            {
            // The metadata is used by a single spatial entity ... we will update the present one ...
            if (ODBCConnectionStatus::Success != (result = ServerConnection::UpdateMetadata(*(data.GetMetadataCP()), metadataId)))
                return result;
            }
        else
            {
            //if (metadataId <= 0)
            //    {
            //    // This means there were no thumbnail previously on this spatial entity ... we will ass a brand new one.
            //    if (ODBCConnectionStatus::Success != (result = ServerConnection::UpdateThumbnail(data.GetThumbnail(), thumbnailId)))
            //        return result;
    
            //    // Now we must set the thumbnail identifier in the spatial entity
            //    retCode = SetMetadataIdToSpatialEntity(entityId, thumbnailId);
    
            //    } 
            //if (uniqueConsumer)
            //    {
            //    // Metadata exist but is used by many spatial entities make use of it ... for the moment we pass
            //    //&&AR What do we do here?
            //    }
            }
        }


    // For every data source we will update existing and add new ones.
    for (size_t index = 0 ; index < data.GetDataSourceCount() ; index++)
        {
        // Check if data source already exists
        if (data.GetDataSource(index).GetLocationInCompound().size() == 0)
            retCode = FetchIdentityFromStringVal(dataSourceId, "SpatialDataSources", "MainURL", data.GetDataSource(index).GetUri().ToString().c_str(), len);
        else
            retCode = FetchIdentityFromDualStringVal(dataSourceId, "SpatialDataSources", "MainURL", data.GetDataSource(index).GetUri().ToString().c_str(), "LocationInCompound", data.GetDataSource(index).GetLocationInCompound().c_str(), len);


        if (ODBCConnectionStatus::Success == result && dataSourceId > 0)
            {
            // Spatial entity already exist ...
            if (ODBCConnectionStatus::Success != (result = ServerConnection::UpdateDataSource(data.GetDataSource(index), dataSourceId)))
                return result;
            }
        else
            {
            // Spatial entity does not exist
            if (ODBCConnectionStatus::Success != (result = ServerConnection::SaveDataSource(data.GetDataSource(index), entityId, data.GetDataSource(index).GetServerId(), dataSourceId)))
                return result;
              }

        BeAssert(dataSourceId != 0);
        }

    return ODBCConnectionStatus::Success;
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
    sprintf(wszInput, "SELECT * FROM [%s].[dbo].[SpatialDataSources] WHERE [MainURL] = '%s'", m_dbName.c_str(), file);

    return HasEntries(wszInput);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
bool ServerConnection::IsMirror(Utf8CP file)
    {
    /*size_t lastPos = file.find_last_of("/\\");
    CHAR wszInput[512];
    sprintf(wszInput, "SELECT * FROM [%s].[dbo].[SpatialDataSources] WHERE CONTAINS([MainURL], '%s'", m_dbName.c_str(), file.substr(lastPos + 1));

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
// @bsimethod                                   Spencer.Mason            	    /2016
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