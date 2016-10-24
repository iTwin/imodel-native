/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/AwsTraverser/AwsTraverser.cpp $
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
#include <RealityPlatform/RealityDataDownload.h>
#include "AwsTraverser.h"
#include "../ODBCSQLConnection.h"

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

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
float FindSize(std::string html, std::string lookFor)
    {
    std::string relevantLine;
    size_t start, stop;

    start = html.find(lookFor);
    if(start == std::string::npos)
        return -10.0f;
    relevantLine = html.substr(start);
    start = relevantLine.find("(");
    if (start == std::string::npos)
        return -10.0f;
    start += 1;

    stop = relevantLine.find("MB)");
    if(stop != std::string::npos)
        {
        relevantLine = relevantLine.substr(start, (stop -start));
        return std::stof(relevantLine) * 1000;
        }
    
    stop = relevantLine.find("KB)");
    if (stop != std::string::npos)
        {
        relevantLine = relevantLine.substr(start, (stop - start));
        return std::stof(relevantLine);
        }

    stop = relevantLine.find("GB)");
    if (stop != std::string::npos)
        {
        relevantLine = relevantLine.substr(start, (stop - start));
        return std::stof(relevantLine) * 1000000;
        }

    return -10.0f;
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
    std::cout << "  -l, --logfile           The file in which to write failed entries" <<std::endl;
    std::cout << "  -h, --help              Show this help message and exit" << std::endl;
    std::cout << "  -u, --update            Enable update mode" << std::endl;
    std::cout << "  -provider:PROVIDER      Set provider name" << std::endl;
    std::cout << "  -cs, --connectionString Connection string to connect to the db (Required)" << std::endl;
    std::cout << "  if there are spaces in an argument, surround it with \"\" " << std::endl;
    std::cout << "  as in \"-cs:Driver={SQL Server}(...)\" " << std::endl;

    std::cout << std::endl << "Press any key to exit." << std::endl;
    getch();
    }

void AwsLogger::Log(std::string message)
    {
    std::cout << message << std::endl;
    }

bool AwsLogger::ValidateLine(size_t lineIndex, std::string line)
    {
    if(0 == lineIndex)
        {
        Log("invalid line: " + line);
        return false;
        }
    return true;
    }

void AwsFileLogger::Log(std::string message)
    {
    logFile << message << std::endl;
    }

void AwsFileLogger::Close()
    {
    logFile.close();
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
    std::string fileName, logFileName;
    bool logErrors = false;
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
        else if (strstr(argv[i], "-l:") || strstr(argv[i], "--logfile:"))
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            logFileName = std::string(substringPosition);
            logErrors = true;
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
    AwsLogger* log;
    if(logErrors)
        {
        log = new AwsFileLogger();
        ((AwsFileLogger*)log)->logFile.open(logFileName);
        if(!((AwsFileLogger*)log)->logFile.is_open())
            {
            log = new AwsLogger();
            log->Log("could not open logfile, proceeding without logging");
            }
        }
    else 
        log = new AwsLogger();


    if(file.is_open())
        {
        ServerConnection& serverConnection = ServerConnection::GetInstance();

        serverConnection.SetStrings(dbName.c_str(), pwszConnStr.c_str());

        getline(file, line); //header

        getline(file, line);

        SQLINTEGER serverId = serverConnection.SaveServer("https://s3-us-west-2.amazonaws.com/landsat-pds/L8/");

        size_t comma;
        std::string id, rest, downloadUrl;
        float cloudCover, min_lat, min_lon, max_lat, max_lon;
        size_t idx;
        AwsPinger& pinger = AwsPinger::GetInstance();
        SpatialEntityDataPtr data;
        float redSize = 0;
        float blueSize = 0;
        float greenSize = 0;
        float panSize = 0;

        do {
            comma = line.find(",");
            if(!log->ValidateLine(comma, line))
                continue;

            id = line.substr(0, comma);
            if(!log->ValidateLine(id.size(), line))
                continue;
            comma ++;
            rest = line.substr(comma);

            if(serverConnection.CheckExists(Utf8String(id.c_str())))
                {   
                log->Log("duplicate: " + line);
                continue;
                }

            comma = rest.find(",");
            if(!log->ValidateLine(comma, line))
                continue;

            comma++;
            rest = rest.substr(comma); //acquisitionDate

            comma = rest.find(",");
            if(!log->ValidateLine(comma, line))
                continue;
            cloudCover = std::stof(rest.substr(0, comma), &idx); //convert
            comma++;
            rest = rest.substr(comma); 

            comma = rest.find(",");
            if(!log->ValidateLine(comma, line))
                continue;
            comma++;
            rest = rest.substr(comma); //processingLevel

            comma = rest.find(",");
            if(!log->ValidateLine(comma, line))
                continue;
            comma++;
            rest = rest.substr(comma); //path

            comma = rest.find(",");
            if(!log->ValidateLine(comma, line))
                continue;
            comma++;
            rest = rest.substr(comma); //row

            comma = rest.find(",");
            if(!log->ValidateLine(comma, line))
                continue;
            min_lat = std::stof(rest.substr(0, comma), &idx);
            comma++;
            rest = rest.substr(comma);

            comma = rest.find(",");
            if(!log->ValidateLine(comma, line))
                continue;
            min_lon = std::stof(rest.substr(0, comma), &idx);
            comma++;
            rest = rest.substr(comma);

            comma = rest.find(",");
            if(!log->ValidateLine(comma, line))
                continue;
            max_lat = std::stof(rest.substr(0, comma), &idx);
            comma++;
            rest = rest.substr(comma);

            comma = rest.find(",");
            if(!log->ValidateLine(comma, line))
                continue;
            max_lon = std::stof(rest.substr(0, comma), &idx);
            comma++;
            rest = rest.substr(comma);

            comma = rest.find(",");
            if(!log->ValidateLine(comma, line))
                continue;
            downloadUrl = rest.substr(0, comma);
            comma++;
            rest = rest.substr(comma);
            if(!log->ValidateLine(downloadUrl.size(), line))
                continue;

            Utf8CP url = downloadUrl.c_str();

            redSize = 0;
            blueSize = 0;
            greenSize = 0;
            panSize = 0;

            pinger.ReadPage(url, redSize, greenSize, blueSize, panSize);

            if(redSize > 0 && blueSize > 0 && greenSize > 0 && panSize > 0)
                {
                //data = new AwsData(id, downloadUrl, cloudCover, DRange2d::From(min_lon, min_lat, max_lon, max_lat), redSize, greenSize, blueSize, panSize, serverId);
                data = SpatialEntityData::Create();
                data->SetName(Utf8CP(id.c_str()));
                data->SetMultibandUrl(Utf8String(downloadUrl.c_str()));
                data->SetCloudCover(cloudCover);
                data->SetFootprintExtents(DRange2d::From(min_lon, min_lat, max_lon, max_lat));
                data->SetIsMultiband(true);
                data->SetRedBandSize(redSize);
                data->SetGreenBandSize(greenSize);
                data->SetBlueBandSize(blueSize);
                data->SetPanchromaticBandSize(panSize);
                data->SetMultibandServerId(serverId);

                serverConnection.Save(*data, false);
                }
            else 
                log->Log("missing files: " + line);

            }while(getline(file, line));
        }
    else
        {
        std::cout << "Unable to open file. Press any key to exit." << std::endl;
        getch();
        return 0;
        }
    log->Close();

    return 1;
}

END_BENTLEY_REALITYPLATFORM_NAMESPACE

int main(int argc, char* argv[])
    {
    return RealityPlatform::main(argc, argv);
    }


