/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/WebTraverser/WebTraverser.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
#include "WebTraverser.h"
#include <RealityAdmin/ODBCSQLConnection.h>
#include <RealityAdmin/FtpTraversalEngine.h>
#include <RealityAdmin/HttpTraversalEngine.h>


BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE


//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void ShowUsage()
    {
    std::cout << "Usage: webtraverser.exe StartUrl [DualStartUrl] [options]" << std::endl <<std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help                      Show this help message and exit" << std::endl;
    std::cout << "  -u, --update                    Enables update mode" << std::endl;
    std::cout << "  -provider:PROVIDER              Sets provider keyname" << std::endl;
    std::cout << "  -providerName:PROVIDERNAME      Sets provider name. If not provided then provider keyname will be used." << std::endl;
    std::cout << "  -legal:LEGAL          Sets metadata terms of use." << std::endl;
    std::cout << "  -termsOfUse:TERMSOFUSE          Sets metadata terms of use." << std::endl;
    std::cout << "  -description:DESCRIPTION        Sets metadata description." << std::endl;
    std::cout << "  -filePattern:FILEPATTERN        Sets the file pattern to filter out files. (See below for details)" << std::endl;
    std::cout << "  -dataset:DATASET                Sets dataset name" << std::endl;
    std::cout << "  -classification:CLASSIFICATION  Sets the classification. If absent then the software will try to determine the classification from file extension." << std::endl;
    std::cout << "  -thumbnails                     Indicates thumbnails should be extracted and set" << std::endl;
    std::cout << "  -cs, --connectionString Connection string to connect to the db (Required)" << std::endl;
    std::cout << "  if there are spaces in an argument, surround it with \"\" " << std::endl;
    std::cout << "  as in \"-cs:Driver={SQL Server}(...)\" " << std::endl;
    std::cout << "  Here follows a full example of a valid ODBC connection string:" << std::endl;
    std::cout << "  -cs:Driver=\"{SQL Server};Server=NAOU10922QBC\\SQLSERVER2012;Database=FurAlain;\" " << std::endl;
    std::cout << "    Note the presence of double-quotes since the driver name contains spaces; these may be required when invoquing from a command line." << std::endl;
    std::cout << "    where the parameter Driver designates the ODBC driver used to access the database, usually SQL Server for this program" << std::endl;
    std::cout << "    Server= is the full server name including the computer name followed by the database engine" << std::endl;
    std::cout << "    and the value of Database is the name of the database to populate inside the engine." << std::endl;
    std::cout << "    additional parameters such as password may be required when integrated security is not used." << std::endl;
    std::cout << "    Refer to ODBC connection string documentation or technical support personnel for additional details." << std::endl;
    std::cout << "  NOTE:" << std::endl;
    std::cout << "     The file pattern applies to files that are processed but necessarily to files downloaded." << std::endl;
    std::cout << "     All archive files will be processed as if they were directories. This implies" << std::endl;
    std::cout << "     that .zip, .gz .7z files will always be downloaded though only files with the specified extension" << std::endl;
    std::cout << "     inside the archive will be processed as spatial entities." << std::endl;
    std::cout << "     The file pattern follows the usual pattern where various extensions can be cumulated by separating" << std::endl;
    std::cout << "     with a semi-comma. The following are all valid file patterns: *.tif *.jpg;*.tif;*.hgt." << std::endl;
    std::cout << "  NOTE:" << std::endl;
    std::cout << "     Classification possible are 'Imagery', 'Model' and 'Terrain' only." << std::endl;



    std::cout << std::endl << "Press any key to exit." << std::endl;
    getch();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
Utf8CP EnumString(SpatialEntityHandlerStatus status)
    {
    switch (status)
        {
        case SpatialEntityHandlerStatus::Success:
            return "Success";
        case SpatialEntityHandlerStatus::ClientError:
            return "Client Error";
        case SpatialEntityHandlerStatus::CurlError:
            return "Curl Error";
        case SpatialEntityHandlerStatus::DataExtractError:
            return "Data Extract Error";
        case SpatialEntityHandlerStatus::DownloadError:
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
    SetConsoleTitle((LPCTSTR)"Web Traversal Engine");

    auto argIt = argv;
    int UrlCount = 0;
    char ftp[10] = "ftp://";
    char http[10] = "http://";
    char https[10] = "https://";
    for (int i = 0; i < argc - 1; ++i)
        {
        char* input = *++argIt;
        if (strstr(input, ftp) != nullptr || strstr(input, http) != nullptr || strstr(input, https) != nullptr )
            UrlCount++;
        }

    if (1 > UrlCount || 2 < UrlCount)
        {
        ShowUsage();

        return 0;
        }

    bool dualMode = (2 == UrlCount);
    bool updateMode = false;
    std::string provider;
    std::string providerName;
    std::string metadataDescription;
    std::string metadataLegal;
    std::string metadataTermsOfUse;

    std::string dataset;
    std::string filePattern = "*";
    std::string classification;
    bool extractThumbnails = false;
    std::vector<std::string> Urls = std::vector<std::string>(UrlCount);
    char* substringPosition;
    std::string dbName;
    std::string pwszConnStr;
    bool hasCString = false;
    Urls.clear();
    for (int i = 0; i < argc; ++i)
        {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            {
            ShowUsage();
            return 0;
            }
        else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--update") == 0)
            updateMode = true;
        else if (strstr(argv[i], "-provider:"))
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            provider = std::string(substringPosition);
            }
        else if (strstr(argv[i], "-providerName:"))
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            providerName = std::string(substringPosition);
            }
        else if (strstr(argv[i], "-filePattern:"))
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            filePattern = std::string(substringPosition);
            }
        else if (strstr(argv[i], "-dataset:"))
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            dataset = std::string(substringPosition);
            }
        else if (strstr(argv[i], "-thumbnail"))
            {
            extractThumbnails = true;
            }
        else if (strstr(argv[i], "-classification"))
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            classification = std::string(substringPosition);
            }
        else if (strstr(argv[i], "-description"))
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            metadataDescription = std::string(substringPosition);
            }
        else if (strstr(argv[i], "-legal"))
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            metadataLegal = std::string(substringPosition);
            }
        else if (strstr(argv[i], "-termsOfUse"))
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            metadataTermsOfUse = std::string(substringPosition);
            }
        else if (strstr(argv[i], "ftp://"))
            Urls.push_back(std::string(argv[i]));        
        else if (strstr(argv[i], "http://"))
            Urls.push_back(std::string(argv[i]));
        else if (strstr(argv[i], "https://"))
            Urls.push_back(std::string(argv[i]));
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

    SpatialEntityHandlerStatus status = SpatialEntityHandlerStatus::UnknownError;
    SpatialEntityClientPtr client = nullptr;
    for (int i = 0; i < UrlCount; ++i)
        {
        try
            {
            std::cout << std::endl << "*****************" << std::endl;
            std::cout << "Connecting to Web" << std::endl;
            std::cout << "*****************" << std::endl << std::endl;

            if (providerName.size() == 0)
                providerName = provider;

            SpatialEntityMetadataPtr metadataSeed = SpatialEntityMetadata::Create();;
            if (metadataDescription.size() > 0 || metadataLegal.size() > 0 || metadataTermsOfUse.size() > 0)
                {
                metadataSeed->SetDescription(metadataDescription.c_str());
                metadataSeed->SetLegal(metadataLegal.c_str());
                metadataSeed->SetTermsOfUse(metadataTermsOfUse.c_str());
                }

            if (strstr(Urls[i].c_str(), ftp) != nullptr)
                client = FtpClientPtr(FtpClient::ConnectTo((Utf8CP)Urls[i].c_str(), (Utf8CP)provider.c_str(), (Utf8CP)providerName.c_str(), (Utf8CP)dataset.c_str(), (Utf8CP)filePattern.c_str(), extractThumbnails, (Utf8CP)classification.c_str(), *(metadataSeed.get())));
            else
                client = HttpClientPtr(HttpClient::ConnectTo((Utf8CP)Urls[i].c_str(), (Utf8CP)provider.c_str(), (Utf8CP)providerName.c_str(), (Utf8CP)dataset.c_str(), (Utf8CP)filePattern.c_str(), extractThumbnails, (Utf8CP)classification.c_str(), *(metadataSeed.get())));

            if (client == nullptr)
                {
                std::cout << "Status: Could not connect to " << Urls[i].c_str() << std::endl;
                continue;
                }
            std::cout << "Status: Connected to " << Urls[i].c_str();

            std::cout << std::endl << "*****************" << std::endl;
            std::cout << "Retrieving data" << std::endl;
            std::cout << "*****************" << std::endl << std::endl;

            client->SetObserver(new WebTraversalObserver(updateMode, dualMode, dbName.c_str(), pwszConnStr.c_str(), true));
            status = client->GetData();
            if (status != SpatialEntityHandlerStatus::Success)
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
WebTraversalObserver::WebTraversalObserver(bool updateMode, bool dualMode, const char* dbName, const char* pwszConnStr, bool verbose) : ISpatialEntityTraversalObserver(), m_updateMode(updateMode), m_dualMode(dualMode), m_verbose(verbose)
    {
    ServerConnection::GetInstance().SetStrings(dbName, pwszConnStr);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void WebTraversalObserver::OnFileListed(bvector<Utf8String>& fileList, Utf8CP file)
    {
    if (nullptr == file)
        {
        if (m_verbose)
            std::cout << "Status: Failed, file is null." << std::endl;
        return;
        }

    if (m_updateMode)
        {
        if (!ServerConnection::GetInstance().IsDuplicate(file))
            {
            if (m_verbose)
                std::cout << "Status: Skipped " << file << std::endl;
            return;
            }
        }
    else
        {
        if (ServerConnection::GetInstance().IsDuplicate(file))
            {
            if (m_verbose)
                std::cout << "Status: Skipped " << file << std::endl;
            return;
            }
        }

    if (m_verbose)
        std::cout << "Status: Added " << file << " to queue." << std::endl;

    fileList.push_back(file);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void WebTraversalObserver::OnFileDownloaded(Utf8CP file)
    {
    if (nullptr == file)
        return;

    if (m_verbose)
        std::cout << "Status: Downloaded " << file << std::endl;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void WebTraversalObserver::OnDataExtracted(RealityPlatform::SpatialEntityCR data)
    {
    for (size_t index = 0 ; index < data.GetDataSourceCount() ; index++)
        {
        if (data.GetDataSource(index).GetServerCP() != NULL)
            data.GetDataSource(index).SetServerId(ServerConnection::GetInstance().SaveServer(*(data.GetDataSource(index).GetServerCP())));
        }
       
    
    if (m_updateMode)
        {
        ServerConnection::GetInstance().Update(data);
        if (m_verbose)
            std::cout << "Status: Database Updated " << data.GetName() << std::endl;
        }
    else
        {
        ServerConnection::GetInstance().SaveSpatialEntity(data, m_dualMode);
        if (m_verbose)
            std::cout << "Status: Database Saved " << data.GetName() << std::endl;
        }
    }

END_BENTLEY_REALITYPLATFORM_NAMESPACE

int main(int argc, char* argv[])
{
    return RealityPlatform::main(argc, argv);
}


