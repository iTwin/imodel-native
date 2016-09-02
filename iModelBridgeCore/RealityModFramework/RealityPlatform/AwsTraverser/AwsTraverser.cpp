/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/AwsTraverser/AwsTraverser.cpp $
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
#include <fstream>
#include <sstream>
#include <vector>
#include <curl/curl.h>
#include <RealityPlatform/HttpTraversalEngine.h>
#include <RealityPlatform/AwsTraverser/AwsTraverser.h>
#include <RealityPlatform/RealityDataDownload.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct CurlHolder
    {
    private:
        CURL* m_curl;

    public:
        CurlHolder() : m_curl(curl_easy_init()) {}
        ~CurlHolder() { if (NULL != m_curl) curl_easy_cleanup(m_curl); }
        CURL* Get() const { return m_curl; }
    };

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
ServerConnection::ServerConnection()
    {
    s_instance = this;
    }

ServerConnection* ServerConnection::s_instance = nullptr;
//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
ServerConnection& ServerConnection::GetInstance()
    {
    if (nullptr == s_instance)
        s_instance = new ServerConnection();
    return *s_instance;
    }

AwsPinger* AwsPinger::s_instance = nullptr;
//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
AwsPinger& AwsPinger::GetInstance()
{
    if (nullptr == s_instance)
        s_instance = new AwsPinger();
    return *s_instance;
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
AwsData::AwsData(std::string id, std::string downloadUrl, float cloudCover, DRange2d ftPrint, 
    float red, float green, float blue, float pan, SQLINTEGER sId, SQLINTEGER mId) :
m_id(id), m_downloadUrl(downloadUrl), m_cloudCover(cloudCover), m_ftPrint(ftPrint),
m_redSize(red), m_greenSize(green), m_blueSize(blue), m_panSize(pan), serverId(sId), metadataId(mId)
{}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
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
// @bsimethod                                   Spencer.Mason            	    9/2016
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
// @bsimethod                                   Spencer.Mason            	    9/2016
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
// @bsimethod                                   Spencer.Mason            	    9/2016
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
// @bsimethod                                   Spencer.Mason            	    9/2016
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
// @bsimethod                                   Spencer.Mason            	    9/2016
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
float FindSize(std::string html, std::string lookFor)
    {
    std::string relevantLine;
    size_t start, stop;

    start = html.find(lookFor);
    relevantLine = html.substr(start);
    start = relevantLine.find("(") + 1;
    stop = relevantLine.find("MB)");
    relevantLine = relevantLine.substr(start, (stop -start));
    return std::stof(relevantLine);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
size_t ParseXML(char* buf, size_t size, size_t nmemb, void* up)
    {
    std::string html = std::string(buf);

    AwsPinger::GetInstance().m_blueSize = FindSize(html, "_B2.TIF\"");
    AwsPinger::GetInstance().m_greenSize = FindSize(html, "_B3.TIF\"");
    AwsPinger::GetInstance().m_redSize = FindSize(html, "_B4.TIF\"");
    AwsPinger::GetInstance().m_panSize = FindSize(html, "_B8.TIF\"");
    
    return size * nmemb;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
void AwsPinger::ReadPage(Utf8CP url, float& redSize, float& greenSize, float& blueSize, float& panSize)
    {
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1); // Verify the SSL certificate.
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 1);
    curl_easy_setopt(m_curl, CURLOPT_URL, url);
    curl_easy_setopt(m_curl, CURLOPT_CAINFO, m_certificatePath);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &ParseXML);
    //curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L); //tell curl to output its progress

    curl_easy_perform(m_curl);

    curl_easy_reset(m_curl);

    redSize = m_redSize;
    blueSize = m_blueSize;
    greenSize = m_greenSize;
    panSize = m_panSize;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
AwsPinger::AwsPinger()
    {
    // Set certificate path.
    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);

    BeFileName caBundlePath(exeDir);
    caBundlePath.AppendToPath(L"Assets").AppendToPath(L"http").AppendToPath(L"cabundle.pem");

    // Make sure directory exist.
    if (caBundlePath.DoesPathExist())
        m_certificatePath = caBundlePath.GetNameUtf8();
    else
    {
        std::cout << "symlink to cabundle.pem was not found in " << caBundlePath.GetName() << std::endl;
        getch();
        exit(-1);
    }

    curl_global_init(CURL_GLOBAL_ALL); 
    m_curl = curl_easy_init();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
AwsPinger::~AwsPinger()
    {
    curl_easy_cleanup(m_curl);
    curl_global_cleanup();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
void ShowUsage()
    {
    std::cout << "Usage: awstraverser.exe -f:[file] -cs:[connectionString] [options]" << std::endl << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -f, --file              The AWS file to parse (Required)" << std::endl;
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
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
int main(int argc, char *argv[])
    {
    SetConsoleTitle((LPCTSTR)"AWS Traversal Engine");

    //auto argIt = argv;

    bool updateMode = false;
    std::string provider;
    char* substringPosition;
    std::string dbName;
    std::string pwszConnStr;
    std::string fileName;
    int hasRequired = 0;
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
        else if (strstr(argv[i], "-f:") || strstr(argv[i], "--file:"))
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            fileName = std::string(substringPosition);
            hasRequired |= 1;
            }
        else if (strstr(argv[i], "--connectionString:") || strstr(argv[i], "-cs:"))
            {
            std::string argument = std::string(argv[i]);
            size_t index = argument.find(":");
            substringPosition = argv[i] + index;
            substringPosition++;
            pwszConnStr = std::string(substringPosition);

            size_t dbIndex = argument.find("Database=");
            std::string dbWithExtra = argument.substr(dbIndex + 9);
            dbIndex = dbWithExtra.find(";");
            dbName = dbWithExtra.substr(0, dbIndex);

            hasRequired |= 2;
            }
        }

    if (!(hasRequired & 1) || !(hasRequired & 2))
        {
        ShowUsage();
        return 0;
        }

    std::string line;
    std::ifstream file(fileName);

    if(file.is_open())
        {
        ServerConnection::GetInstance().SetStrings(dbName.c_str(), pwszConnStr.c_str());

        getline(file, line); //header

        getline(file, line);

        SQLINTEGER serverId = ServerConnection::GetInstance().SaveServer("https://s3-us-west-2.amazonaws.com/landsat-pds/L8/");
        SQLINTEGER metadataId = ServerConnection::GetInstance().SaveMetadata();

        size_t comma;
        std::string id, rest, downloadUrl;
        float cloudCover, min_lat, min_lon, max_lat, max_lon;
        size_t idx;
        AwsPinger& pinger = AwsPinger::GetInstance();
        AwsData* data;

        do {
            comma = line.find(",");
            id = line.substr(0, comma);
            comma ++;
            rest = line.substr(comma);

            comma = rest.find(",");
            comma++;
            rest = rest.substr(comma); //acquisitionDate

            comma = rest.find(",");
            cloudCover = std::stof(rest.substr(0, comma), &idx); //convert
            comma++;
            rest = rest.substr(comma); 

            comma = rest.find(",");
            comma++;
            rest = rest.substr(comma); //processingLevel

            comma = rest.find(",");
            comma++;
            rest = rest.substr(comma); //path

            comma = rest.find(",");
            comma++;
            rest = rest.substr(comma); //row

            comma = rest.find(",");
            min_lat = std::stof(rest.substr(0, comma), &idx);
            comma++;
            rest = rest.substr(comma);

            comma = rest.find(",");
            min_lon = std::stof(rest.substr(0, comma), &idx);
            comma++;
            rest = rest.substr(comma);

            comma = rest.find(",");
            max_lat = std::stof(rest.substr(0, comma), &idx);
            comma++;
            rest = rest.substr(comma);

            comma = rest.find(",");
            max_lon = std::stof(rest.substr(0, comma), &idx);
            comma++;
            rest = rest.substr(comma);

            comma = rest.find(",");
            downloadUrl = rest.substr(0, comma);
            comma++;
            rest = rest.substr(comma);

            Utf8CP url = downloadUrl.c_str();

            float redSize = 0;
            float blueSize = 0;
            float greenSize = 0;
            float panSize = 0;

            pinger.ReadPage(url, redSize, blueSize, greenSize, panSize);

            data = new AwsData(id, downloadUrl, cloudCover, DRange2d::From(min_lat, min_lon, max_lat, max_lon), redSize, greenSize, blueSize, panSize, serverId, metadataId);

            ServerConnection::GetInstance().Save(*data);

            }while(getline(file, line));
        }
    else
        {
        std::cout << "Unable to open file. Press any key to exit." << std::endl;
        getch();
        return 0;
        }
    
    return 1;
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
SQLINTEGER ServerConnection::SaveServer(std::string url)
    {
    CHAR preQuery[512];
    sprintf(preQuery, "SELECT * FROM [%s].[dbo].[Servers] WHERE [URL] = '%s'", m_dbName, url.c_str());
    CHAR serverStatement[512];
    if (HasEntries(preQuery))
        {
        sprintf(serverStatement, "UPDATE [%s].[dbo].[Servers] SET [LastCheck] = ?, [LastTimeOnline] = ? WHERE [URL] = '%s'",
            m_dbName,
            url.c_str());
        }
    else
        {
        sprintf(serverStatement, "INSERT INTO [%s].[dbo].[Servers] ([CommunicationProtocol], [Name], [URL], [Online], [LastCheck], [LastTimeOnline]) VALUES ('http', 's3-us-west-2.amazonaws.com', '%s', 1, ?, ?)",
            m_dbName,
            url.c_str());
        }

    SQLPrepare(hStmt, (SQLCHAR*)serverStatement, SQL_NTS);
    DateTime dateTime = DateTime::GetCurrentTimeUtc();

    SQL_TIMESTAMP_STRUCT checkTime = PackageDateTime(dateTime);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TYPE_TIMESTAMP, 23, 3, &checkTime, sizeof(SQL_TIMESTAMP_STRUCT), 0);
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TYPE_TIMESTAMP, 23, 3, &checkTime, sizeof(SQL_TIMESTAMP_STRUCT), 0);

    ExecuteSQL(hStmt);
    ReleaseStmt();

    SQLINTEGER id;
    SQLLEN len;

    CHAR idQuery[256];
    sprintf(idQuery, "SELECT [ID] FROM [%s].[dbo].[Servers] WHERE [URL] = '%s'", m_dbName, url.c_str());
    ExecuteSQL(idQuery);
    SQLBindCol(hStmt, 1, SQL_INTEGER, &id, 2, &len);
    TryODBC(hStmt, SQL_HANDLE_STMT, SQLFetch(hStmt));
    ReleaseStmt();

    return id;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
SQLINTEGER ServerConnection::SaveMetadata()
{
    CHAR preQuery[512];
    sprintf(preQuery, "SELECT * FROM [%s].[dbo].[Metadatas] WHERE [Provenance] = 'landsat8'", m_dbName);
    CHAR serverStatement[512];
    if (!HasEntries(preQuery))
        {
        sprintf(serverStatement, "INSERT INTO [%s].[dbo].[Metadatas] ([Description], [Provenance]) VALUES ('Landsat data provided by Amazon Web Services', 'landsat8')",
            m_dbName);
        ExecuteSQL(serverStatement);
        ReleaseStmt();
        }

    SQLINTEGER id;
    SQLLEN len;

    CHAR idQuery[256];
    sprintf(idQuery, "SELECT [ID] FROM [%s].[dbo].[Metadatas] WHERE [Provenance] = 'landsat8'", m_dbName);
    ExecuteSQL(idQuery);
    SQLBindCol(hStmt, 1, SQL_INTEGER, &id, 2, &len);
    TryODBC(hStmt, SQL_HANDLE_STMT, SQLFetch(hStmt));
    ReleaseStmt();

    return id;
}

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
void ServerConnection::Save(AwsData awsdata)
    {
    CHAR existsQuery[256];
    sprintf(existsQuery, "SELECT * FROM [%s].[dbo].[MultibandSources] WHERE [OriginalId] = '%s'", 
        m_dbName,
        awsdata.GetId().c_str());
    if(HasEntries(existsQuery))
        return;

    std::string downloadUrl = awsdata.GetUrl();
    size_t ext = downloadUrl.rfind("/");
    std::string baseUrl = downloadUrl.substr(0, ext + 1);
    baseUrl.append(awsdata.GetId());
    std::string thumbUrl = baseUrl;
    thumbUrl.append("_thumb_large.jpg");
    std::string blueUrl = baseUrl;
    blueUrl.append("_B2.TIF");
    std::string greenUrl = baseUrl;
    greenUrl.append("_B3.TIF");
    std::string redUrl = baseUrl;
    redUrl.append("_B4.TIF");
    std::string panUrl = baseUrl;
    panUrl.append("_B8.TIF");

    CHAR thumbnailQuery[1000];
    sprintf(thumbnailQuery, "INSERT INTO [%s].[dbo].[Thumbnails] ([ThumbnailProvenance], [ThumbnailFormat], [ThumbnailStamp], [ThumbnailGenerationDetails], [ThumbnailUrl]) VALUES ('Provided by Amazon Web Services', 'png', '%ls', 'Provided by Amazon Web Services', '%s')",
        m_dbName,
        DateTime::GetCurrentTimeUtc().ToString().c_str(),
        thumbUrl.c_str());

    RETCODE retCode;
    retCode = ExecuteSQL(thumbnailQuery);
    ReleaseStmt();
    SQLINTEGER thumbnailId;
    SQLLEN len;

    CHAR tableName[128];
    sprintf(tableName, "[%s].[dbo].[Thumbnails]", m_dbName);
    FetchTableIdentity(thumbnailId, tableName, len);
    
    SQLINTEGER entityId;

    CHAR entityBaseQuery[2000];
    sprintf(entityBaseQuery, "INSERT INTO [%s].[dbo].[SpatialEntityBases] ([Name], [DataProvider], [DataProviderName], [Footprint], [Date], [Metadata_Id], [Thumbnail_Id]) VALUES ('%s', 'Amazon Landsat 8', 'Amazon Web Services', geometry::STPolyFromText(?, 0), ?, %d, %d)",
        m_dbName,
        awsdata.GetId().c_str(),
        awsdata.GetMetadataId(),
        thumbnailId);

    SQLPrepare(hStmt, (SQLCHAR*)entityBaseQuery, SQL_NTS);

    DRange2dCR Fpt = awsdata.GetFootprint();
    double xMin = std::min(Fpt.low.x, Fpt.high.x);
    double xMax = std::max(Fpt.low.x, Fpt.high.x);
    double yMin = std::min(Fpt.low.y, Fpt.high.y);
    double yMax = std::max(Fpt.low.y, Fpt.high.y);
    char polygon[2000];
    sprintf(polygon, "POLYGON((%f %f, %f %f, %f %f, %f %f, %f %f))", xMax, yMax, xMax, yMin, xMin, yMin, xMin, yMax, xMax, yMax);
    retCode = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_LONGVARCHAR, strlen(polygon), 0, (SQLPOINTER)polygon, strlen(polygon), NULL);

    DateTimeCR date = DateTime::GetCurrentTimeUtc();
    CHAR baseDate[10];
    sprintf(baseDate, "%d-%d-%d", date.GetYear(), date.GetMonth(), date.GetDay());
    TryODBC(hStmt, SQL_HANDLE_STMT, SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, strlen(baseDate), 0, (SQLPOINTER)baseDate, strlen(baseDate), NULL));

    ExecuteSQL(hStmt);
    ReleaseStmt();

    sprintf(tableName, "[%s].[dbo].[SpatialEntityBases]", m_dbName);
    FetchTableIdentity(entityId, tableName, len);

    CHAR spatialDataSourceQuery[512];
    sprintf(spatialDataSourceQuery, "INSERT INTO [%s].[dbo].[SpatialDataSources] ([MainURL], [DataSourceType], [NoDataValue], [FileSize], [Server_Id]) VALUES ('%s', 'TIF', 0, %f, %d)",
        m_dbName,
        awsdata.GetUrl().c_str(),
        awsdata.GetRedSize() + awsdata.GetGreenSize() + awsdata.GetBlueSize() + awsdata.GetPanchromaticSize(),
        awsdata.GetServerId());

    retCode = ExecuteSQL(spatialDataSourceQuery);
    ReleaseStmt();
    SQLINTEGER dataSourceId;

    sprintf(tableName, "[%s].[dbo].[SpatialDataSources]", m_dbName);
    FetchTableIdentity(dataSourceId, tableName, len);

    ReleaseStmt();

    CHAR entityQuery[255];
    sprintf(entityQuery, "INSERT INTO [%s].[dbo].[SpatialEntities] ([Id], [CloudCoverage]) VALUES (%d, %f)",
        m_dbName,
        entityId,
        awsdata.GetCover());
    ExecuteSQL(entityQuery);
    ReleaseStmt();

    CHAR existingSourceQuery[512];
    sprintf(existingSourceQuery, "INSERT INTO [%s].[dbo].[SpatialEntitySpatialDataSources] ([SpatialEntity_Id], [SpatialDataSource_Id]) VALUES (%d, %d)",
        m_dbName,
        entityId,
        dataSourceId);

    ExecuteSQL(existingSourceQuery);
    ReleaseStmt();

    CHAR multiBQuery[1000];
    sprintf(multiBQuery, "INSERT INTO [%s].[dbo].[MultibandSources] ([Id], [OriginalId], [RedBandURL], [RedBandFileSize], [GreenBandURL], [GreenBandFileSize], [BlueBandURL], [BlueBandFileSize], [PanchromaticBandURL], [PanchromaticBandFileSize]) VALUES ( %d, '%s', '%s', %f, '%s', %f, '%s', %f, '%s', %f)",
        m_dbName,
        dataSourceId,
        awsdata.GetId().c_str(),
        redUrl.c_str(),
        awsdata.GetRedSize(),
        greenUrl.c_str(),
        awsdata.GetGreenSize(),
        blueUrl.c_str(),
        awsdata.GetBlueSize(),
        panUrl.c_str(),
        awsdata.GetPanchromaticSize());

    ExecuteSQL(multiBQuery);
    ReleaseStmt();

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
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
// @bsimethod                                   Spencer.Mason            	    9/2016
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
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
bool ServerConnection::IsDuplicate(Utf8CP file)
    {
    CHAR wszInput[512];
    sprintf(wszInput, "SELECT * FROM [%s].[dbo].[SpatialDataSources] WHERE [MainURL] = '%s'", m_dbName, file);

    return HasEntries(wszInput);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    9/2016
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
// @bsimethod                                   Spencer.Mason            	    9/2016
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
// @bsimethod                                   Spencer.Mason            	    9/2016
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

int main(int argc, char* argv[])
    {
    return RealityPlatform::main(argc, argv);
    }


