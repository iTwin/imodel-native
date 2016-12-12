#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <thread>
#include <mutex>
#include <random>
#include <fstream>

#include <Bentley/BeTest.h>
#include <Bentley/BeFile.h>
#include <RealityPlatform/RealityConversionTools.h>
#include <RealityPlatform/RealityDataDownload.h>
#include <BentleyDesktopClient/CCApi/CCPublic.h>

#include "RealityContextTest.h"

#define M_PI                        3.1415926f
///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Francis Boily                   09/2015
//+---------------+---------------+---------------+---------------+---------------+------*/
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
    ((Utf8String*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Francis Boily                   09/2015
//+---------------+---------------+---------------+---------------+---------------+------*/
static size_t WriteData(void *contents, size_t size, size_t nmemb, FILE *stream)
    {
    size_t written = fwrite(contents, size, nmemb, stream);
    return written;
    }

std::mutex innactiveUserMutex;
std::mutex statMutex;
std::mutex rpsMutex;

static bvector<User*> s_innactiveUsers = bvector<User*>();
static RPS s_rps = RPS();
static bool s_keepRunning = true;
static Stats s_stats = Stats();

RPS::RPS():requestLog(bmap<OperationType, bmap<time_t, int>>())
    {
    requestLog.Insert(OperationType::SPATIAL, bmap<time_t, int>());
    requestLog.Insert(OperationType::PACKID, bmap<time_t, int>());
    requestLog.Insert(OperationType::PACKFILE, bmap<time_t, int>());
    }

void RPS::AddRequest(OperationType type, time_t time)
    {
    std::lock_guard<std::mutex> lock(rpsMutex);
    requestLog[type][time] += 1;
    }

double RPS::GetRPS(OperationType type, time_t time)
    {
    //std::lock_guard<std::mutex> lock(rpsMutex); //lock mutex before calling
    bmap<time_t, int> times = requestLog[type];

    int amount = 0;
    for(time_t i = (time - 12); i < (time - 2); i++)
        amount += times[i];
    return amount/10.0;
    }

size_t getInnactiveUserSize()
    {
    std::lock_guard<std::mutex> lock(innactiveUserMutex);
    return s_innactiveUsers.size();
    }

void restartUser(UserManager* manager)
    {
    std::lock_guard<std::mutex> lock(innactiveUserMutex);
    User* user = s_innactiveUsers.back();
    s_innactiveUsers.pop_back();
    user->DoNext(manager);
    }

void Dispatch(UserManager* manager)
    {
    while (s_keepRunning)
        {
        if (getInnactiveUserSize() == 0)
            {
            Sleep(2000);
            continue;
            }

        restartUser(manager);

        int sleep = rand() % 2000;
        if (sleep > 999)
            Sleep(sleep);

        if(s_keepRunning)
            s_stats.PrintStats();
        }
    }

void Terminate()
    {
    getch();
    s_keepRunning = false;
    std::cout << "exit requested, wrapping up pending requests and exiting..." << std::endl;
    }

void Stat::Update(bool isSuccess, time_t time)
    {
    int total = success + failure;
    if(isSuccess)
        success+=1;
    else
        failure+=1;

    minTime = std::min(minTime, time);
    maxTime = std::max(maxTime, time);
    avgTime = ((avgTime*total) + time) / (total+1);
    }

Stats::Stats()
    {   
    opStats = bmap<OperationType, Stat*>();
    opStats.Insert(OperationType::SPATIAL, new Stat());
    opStats.Insert(OperationType::PACKID, new Stat());
    opStats.Insert(OperationType::PACKFILE, new Stat());
    }

void Stats::InsertStats(OperationType type, bool success, time_t time, Utf8String errorMsg)
    {
    std::lock_guard<std::mutex> lock(statMutex);
    opStats[type]->Update(success, time);
    if(!success)
        errors.push_back(errorMsg);
    }

void Stats::PrintStats()
    {
    std::lock_guard<std::mutex> statlock(statMutex);
    std::lock_guard<std::mutex> rpslock(rpsMutex);
    time_t currentTime = std::time(nullptr);
    system("cls");

    std::cout << "Type      Success    Failure   minTime   maxTime   avgTime  requests/second" << std::endl;
    char spatialLine[1024], idLine[1024], fileLine[1024];
    sprintf(spatialLine, "Spatial %6d %10d %9d %10d %9d        %f", opStats[OperationType::SPATIAL]->success, opStats[OperationType::SPATIAL]->failure, (int)opStats[OperationType::SPATIAL]->minTime, (int)opStats[OperationType::SPATIAL]->maxTime, (int)opStats[OperationType::SPATIAL]->avgTime, s_rps.GetRPS(OperationType::SPATIAL, currentTime));
    sprintf(idLine, "PackId  %6d %10d %9d %10d %9d        %f", opStats[OperationType::PACKID]->success, opStats[OperationType::PACKID]->failure, (int)opStats[OperationType::PACKID]->minTime, (int)opStats[OperationType::PACKID]->maxTime, (int)opStats[OperationType::PACKID]->avgTime, s_rps.GetRPS(OperationType::PACKID, currentTime));
    sprintf(fileLine, "Package %6d %10d %9d %10d %9d        %f", opStats[OperationType::PACKFILE]->success, opStats[OperationType::PACKFILE]->failure, (int)opStats[OperationType::PACKFILE]->minTime, (int)opStats[OperationType::PACKFILE]->maxTime, (int)opStats[OperationType::PACKFILE]->avgTime, s_rps.GetRPS(OperationType::PACKFILE, currentTime));
    
    std::cout << spatialLine << std::endl;
    std::cout << idLine << std::endl;
    std::cout << fileLine << std::endl << std::endl;

    std::cout << "Press any key to quit testing" << std::endl;
    }

void Stats::WriteToFile(int userCount)
    {
    time_t generatedFileName = std::time(nullptr);
    char name[64];
    sprintf(name, "%d.log", (int)generatedFileName);
    ofstream file (name);
    file << userCount << " users" << std::endl;
    file << asctime(localtime(&generatedFileName)) << std::endl;
    file << "Type      Success    Failure   minTime   maxTime   avgTime" << std::endl;

    char spatialLine[1024], idLine[1024], fileLine[1024];
    sprintf(spatialLine, "Spatial %6d %10d %9d %10d %9d", opStats[OperationType::SPATIAL]->success, opStats[OperationType::SPATIAL]->failure, (int)opStats[OperationType::SPATIAL]->minTime, (int)opStats[OperationType::SPATIAL]->maxTime, (int)opStats[OperationType::SPATIAL]->avgTime);
    sprintf(idLine, "PackId  %6d %10d %9d %10d %9d", opStats[OperationType::PACKID]->success, opStats[OperationType::PACKID]->failure, (int)opStats[OperationType::PACKID]->minTime, (int)opStats[OperationType::PACKID]->maxTime, (int)opStats[OperationType::PACKID]->avgTime);
    sprintf(fileLine, "Package %6d %10d %9d %10d %9d", opStats[OperationType::PACKFILE]->success, opStats[OperationType::PACKFILE]->failure, (int)opStats[OperationType::PACKFILE]->minTime, (int)opStats[OperationType::PACKFILE]->maxTime, (int)opStats[OperationType::PACKFILE]->avgTime);

    file << spatialLine << std::endl;
    file << idLine << std::endl;
    file << fileLine << std::endl << std::endl << "error list:" << std::endl;

    for(Utf8String error : errors)
        file << error << std::endl;

    file.close();
    }

static const bvector<Region> worldRegions = 
    { 
    Region(22.0,   -77.0,      600.0,      "Free Republic of Cuba"),
    Region(-13.0,  -73.0,      1500.0,     "South America 1"),
    Region(-22.0,  -58.0,      1000.0,     "South America 2"),
    Region(47.0,   -92.0,      2500.0,     "North America"),
    Region(46.0,    7.0,       1000.0,     "Europe 1"),
    Region(55.0,    38.0,      1800.0,     "Europe 2"),
    Region(49.0,    89.0,      3000.0,     "Europe 3"),
    Region(31.0,    108.0,     1300.0,     "China"),
    Region(-26.0,   140.0,     1200.0,     "Australia 1"),
    Region(-26.0,   121.0,     850.0,      "Australia 2"),
    Region(23.0,    78.0,      800.0,      "India")
    };

void User::SelectRegion()
    {
    m_region = worldRegions[rand() % worldRegions.size()];
    }

float User::Degree2Radians(float degree)
    {
    return (degree / 180.0f) * M_PI;
    }

float User::Radians2Degree(float radius)
    {
    return (radius / M_PI) * 180.0f;
    }

void User::SelectExtents()
    {
    float radiusInDegrees = m_region.m_radius * 1000.0f / 111300.0f;

    float u = (float)((*m_distribution)(*m_generator));
    float v = (float)((*m_distribution)(*m_generator));

    float w = radiusInDegrees * sqrt(u);
    float t = 2 * M_PI * v;
    float x1 = w * std::cos(t);
    float y = w * std::sin(t);

    float x = x1 * std::cos(m_region.m_latitude);

    float xLat = x + m_region.m_latitude;
    float yLong = y + m_region.m_longitude;

    float sizeKm = (((float)((*m_distribution)(*m_generator)) * 99.5f) + 0.5f) / 2.0f;

    float lat = Degree2Radians(xLat);

    float earthRadius = 6371.0f;
    float parallel_radius = earthRadius*std::cos(lat);

    float latSize = Radians2Degree(sizeKm / earthRadius);
    float lonSize = Radians2Degree(sizeKm / parallel_radius);

    bvector<GeoPoint2d> filterPolygon = bvector<GeoPoint2d>();
    GeoPoint2d p1, p2, p3, p4, p5;
    p1.Init(xLat - latSize, yLong - lonSize);
    p2.Init(xLat + latSize, yLong - lonSize);
    p3.Init(xLat + latSize, yLong + lonSize);
    p4.Init(xLat - latSize, yLong + lonSize);
    p5.Init(xLat - latSize, yLong - lonSize);
    filterPolygon.push_back(p1);
    filterPolygon.push_back(p2);
    filterPolygon.push_back(p3);
    filterPolygon.push_back(p4);
    filterPolygon.push_back(p5);

    if(m_bench)
        m_bench->SetGeoParam(RealityPlatform::GeoCoordinationParams(filterPolygon, m_serverType));
    }

User::User(Utf8StringP token, std::default_random_engine* generator, std::uniform_real_distribution<double>* distribution, RealityPlatform::ServerType serverType):
    m_token(token), m_currentOperation(OperationType::SPATIAL), m_packageFile(nullptr), m_retryCounter(0),
    m_generator(generator), m_distribution(distribution), m_serverType(serverType)
    {
    SelectRegion();
    m_bench = RealityPlatform::ContextServicesWorkbench::Create("", RealityPlatform::GeoCoordinationParams());
    }

void User::DoNext(UserManager* owner)
    {
    switch(m_currentOperation)
        {
        case OperationType::SPATIAL:
            m_bench->Init();
            SelectExtents();
            owner->SetupCurl(this, m_bench->CreateSpatialEntityWithDetailsViewUrl(), m_bench->GetSpatialEntityWithDetailsJsonPointer());
            break;
        case OperationType::PACKID:
            m_bench->SelectRandomResolution();
            m_packageParameters = m_bench->GetPackageParameters(m_selectedIds);
            owner->SetupCurl(this, m_bench->GetPackageIdUrl(), m_bench->GetPackageIdPointer(), nullptr, m_packageParameters);
            break;
        case OperationType::PACKFILE:
            m_packageFile = m_bench->OpenPackageFile();
            if(!m_packageFile)
                {
                std::cout << "open file failure, aborting user" << std::endl;
                std::lock_guard<std::mutex> lock(innactiveUserMutex);
                s_innactiveUsers.push_back(this);
                break;
                }
            owner->SetupCurl(this, m_bench->GetPackageFileUrl(), nullptr, m_packageFile);
            break;
        /*case OperationType::FILES:
            break;*/
        }
    }

void User::SampleIds(Json::Value regionItems)
    {
    bvector<Utf8String> retVector = bvector<Utf8String>();
    
    for(Json::Value instance : regionItems["instances"])
        {
        size_t size = retVector.size();
        if(size < 1 || ((rand() % (size * size * 2)) < 1)) //exponentially harder to grow list
            retVector.push_back(instance["instanceId"].asString());
        }

    m_selectedIds = retVector;
    }

bool User::ValidateSpatial()
    {
    time_t currentTime = std::time(nullptr);
    bool retval = true;
    Json::Value regionItems(Json::objectValue);
    if (!Json::Reader::Parse(m_bench->GetSpatialEntityWithDetailsJson(), regionItems) || (!regionItems.isMember("errorMessage") && !regionItems.isMember("instances")))
        {
        retval = false;
        //std::cout << "no json" << std::endl;
        }
    else if (regionItems.isMember("errorMessage"))
        {
        retval = false;
        //std::cout << "bad json" << std::endl;
        }

    if(retval)
        {
        if ((rand() % 100) > 4) // 5% chance to do another spatial query
            {
            SampleIds(regionItems);
            if(m_selectedIds.size() > 0)
                m_currentOperation = OperationType::PACKID;
            m_retryCounter = 0;
            }
        }
    
    s_stats.InsertStats(OperationType::SPATIAL, retval, currentTime - m_downloadStart, m_bench->GetSpatialEntityWithDetailsJson());

    return retval;
    }

bool User::ValidatePackageId()
    {
    time_t currentTime = std::time(nullptr);
    bool retval = true;
    Json::Value packageInfos(Json::objectValue);
    Json::Reader::Parse(*(m_bench->GetPackageIdPointer()), packageInfos);

    if (!packageInfos.isMember("changedInstance"))
        {
        retval = false;
        s_stats.InsertStats(OperationType::PACKID, retval, currentTime - m_downloadStart, "packageId: no member changedInstance");
        return retval;
        }

    m_bench->SetInstanceId(packageInfos["changedInstance"]["instanceAfterChange"]["instanceId"].asCString());

    m_currentOperation = OperationType::PACKFILE;
    m_retryCounter = 0;

    s_stats.InsertStats(OperationType::PACKID, retval, currentTime - m_downloadStart);

    return retval;
    }

bool User::ValidatePacakgeFile()
    {
    time_t currentTime = std::time(nullptr);
    bool retval = true;

    fclose(m_packageFile);

    BeFile file;
    file.Open(m_bench->GetPackageFileName().c_str(), BeFileAccess::Read);
    uint64_t packageSize;
    file.GetSize(packageSize);
    if(packageSize < 1)
        retval = false;
    file.Close();

    if (retval)
        {
        m_bench->SetPackageDownloaded(true);

        /*if((rand() % 100) < -1) //innactive for now
            m_currentOperation = OperationType::FILES;
        else
            {*/
        m_currentOperation = OperationType::SPATIAL;
        BeFileName::BeDeleteFile(m_bench->GetPackageFileName().c_str());
        m_retryCounter = 0;
            //}
        }

    s_stats.InsertStats(OperationType::PACKFILE, retval, currentTime - m_downloadStart, "PackageFile: package is empty");

    return retval;
    }

bool User::ValidatePrevious()
    {
    bool retval = true;
    switch (m_currentOperation)
        {
        case OperationType::SPATIAL:
            retval = ValidateSpatial();    
            break;

        case OperationType::PACKID:
            retval = ValidatePackageId();
            break;

        case OperationType::PACKFILE:
            retval = ValidatePacakgeFile();
            break;

        case OperationType::FILES:
            if (retval)
                m_currentOperation = OperationType::SPATIAL;
            break;
        }
    return retval;
    }

BeFileName GetPemLocation()
    {
    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);

    BeFileName caBundlePath(exeDir);
    caBundlePath.AppendToPath(L"Assets").AppendToPath(L"http").AppendToPath(L"ContextServices.pem");
    return caBundlePath;
    }

void ShowUsage()
    {
    std::cout << "Usage: RealityContextTest.exe -s:[serverType] -u:[users]" << std::endl << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help              Show this help message and exit" << std::endl;
    std::cout << "  -s, --serverType        {dev, qa, prod}" << std::endl;
    std::cout << "  -u, --users             Number of users" << std::endl;
    std::cout << "  if there are spaces in an argument, surround it with \"\" " << std::endl;

    std::cout << std::endl << "Press any key to exit." << std::endl;
    getch();
    }

int main(int argc, char* argv[])
    {
    CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
    CallStatus status = APIERR_SUCCESS;

    bool installed;
    status = CCApi_IsInstalled(api, &installed);
    if (!installed)
        {
        cout << "Client not installed, or COM not registered" << endl;
        ShowUsage();
        }
    bool running = false;
    status = CCApi_IsRunning(api, &running);
    if (status != APIERR_SUCCESS)
        {
        wprintf(L"IsRunning failed with error code %d\n", status);
        ShowUsage();
        }
    if (!running)
        {
        cout << "Client not running" << endl;
        ShowUsage();
        }
    bool loggedIn = false;
    status = CCApi_IsLoggedIn(api, &loggedIn);
    if (status != APIERR_SUCCESS)
        {
        wprintf(L"IsLoggedIn failed with error code %d\n", status);
        ShowUsage();
        }
    if (!loggedIn)
        {
        cout << "Client not logged in" << endl;
        ShowUsage();
        }
    bool acceptedEula = false;
    status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
    if (status != APIERR_SUCCESS)
        {
        wprintf(L"CCApi_HasUserAcceptedEULA failed with error code %d\n", status);
        ShowUsage();
        }
    if (!acceptedEula)
        {
        cout << "Client has not accepted the EULA" << endl;
        return -1;
        }
    bool sessionActive = false;
    status = CCApi_IsUserSessionActive(api, &sessionActive);
    if (status != APIERR_SUCCESS)
        {
        wprintf(L"CCApi_IsUserSessionActive failed with error code %d\n", status);
        ShowUsage();
        }
    if (!sessionActive)
        {
        cout << "Session not active" << endl;
        ShowUsage();
        }

    LPCWSTR relyingParty = L"https://connect-wsg20.bentley.com";
    UINT32 maxTokenLength = 16384;
    LPWSTR lpwstrToken = new WCHAR[maxTokenLength];

    status = CCApi_GetSerializedDelegateSecurityToken(api, relyingParty, lpwstrToken, maxTokenLength);
    if (status != APIERR_SUCCESS)
        {
        wprintf(L"CCApi_GetSerializedDelegateSecurityToken failed with error code %d\n", status);
        ShowUsage();
        }

    char* charToken = new char[maxTokenLength];
    wcstombs(charToken, lpwstrToken, maxTokenLength);

    Utf8String token = "Authorization: Token ";
    token.append(charToken);

    char* substringPosition;
    int userCount = 0;
    RealityPlatform::ServerType serverType = RealityPlatform::ServerType::QA;

    if(argc != 3)
        {
        ShowUsage();
        return 0;
        }

    for (int i = 0; i < argc; ++i)
        {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            {
            ShowUsage();
            return 0;
            }
        else if (strstr(argv[i], "-u") || strstr(argv[i], "--users"))
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            Utf8String userString = Utf8String(substringPosition);
            uint64_t users;
            BeStringUtilities::ParseUInt64(users, userString.c_str());
            userCount = (int)users;
            }
        else if (strstr(argv[i], "-s:") || strstr(argv[i], "--serverType:"))
            {
            if(strstr(argv[i], "qa"))
                serverType = RealityPlatform::ServerType::QA;
            else if (strstr(argv[i], "prod"))
                serverType = RealityPlatform::ServerType::PROD;
            else
                serverType = RealityPlatform::ServerType::DEV;
            }
        }
    
    UserManager wo = UserManager();
    wo.m_certPath = GetPemLocation();

    for(int i = 0; i < userCount; i++)
        {
        wo.users.push_back(new User(&token, &wo.m_generator, &wo.m_distribution, serverType));
        }

    std::thread dispatch (Dispatch, &wo);
    std::thread terminate (Terminate);

    wo.Perform(userCount);
    s_keepRunning = false;
    terminate.join();
    dispatch.join();
    s_stats.WriteToFile(userCount);
    }

UserManager::UserManager()
    {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    m_pCurlHandle = curl_multi_init();
    m_certPath = WString();
    m_distribution = std::uniform_real_distribution<double>(0.0, 1.0);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Donald.Morissette  9/2015
//----------------------------------------------------------------------------------------
UserManager::~UserManager()
    {
    if (m_pCurlHandle != NULL)
        curl_multi_cleanup(m_pCurlHandle);
    curl_global_cleanup();
    for(int i = 0; i < users.size(); i++)
        delete users[i];
    }

User::~User()
    {
    delete m_bench;
    }

void UserManager::Perform(int userCount)
    {
    // we can optionally limit the total amount of connections this multi handle uses 
    curl_multi_setopt(m_pCurlHandle, CURLMOPT_MAXCONNECTS, userCount);

    for (int i = 0; i < min(userCount, (int)users.size()); ++i)
        {
        //SetupCurl(users[i]);
        users[i]->DoNext(this);
        }
    
    int still_running; /* keep number of running handles */
    int repeats = 0;

    std::cout << "launching requests, it may take a few seconds to receive responses and display results" << std::endl;

    //int failCounter = 0;
    do
        {
        CURLMcode mc; /* curl_multi_wait() return code */
        int numfds;

        curl_multi_perform(m_pCurlHandle, &still_running);

        /* wait for activity, timeout or "nothing" */
        mc = curl_multi_wait(m_pCurlHandle, NULL, 0, 1000, &numfds);

        if (mc != CURLM_OK)
            {
            std::cout << "curl_multi_wait() failed" << std::endl;
            break;
            }

        /* 'numfds' being zero means either a timeout or no file descriptors to
        wait for. Try timeout on first occurrence, then assume no file
        descriptors and no file descriptors to wait for means wait for 100
        milliseconds. */

        if (!numfds)
            {
            repeats++; /* count number of repeated zero numfds */
            if (repeats > 1)
                Sleep(300); /* sleep 100 milliseconds */
            }
        else
            repeats = 0;

        CURLMsg *msg;
        int nbQueue;
        while ((msg = curl_multi_info_read(m_pCurlHandle, &nbQueue)))
            {
            if (msg->msg == CURLMSG_DONE)
                {
                char *pClient;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &pClient);
                struct User *user = (struct User *)pClient;

                if (msg->data.result == CURLE_OK)
                    {
                    /*if(!user->ValidatePrevious())
                        failCounter++;*/
                    user->ValidatePrevious();

                    if(s_keepRunning)
                        {
                        int keepGoing = rand() % 100;
                        if((user->m_currentOperation == OperationType::PACKFILE) || keepGoing < 20)
                            user->DoNext(this);
                        else
                            {
                            std::lock_guard<std::mutex> lock(innactiveUserMutex);
                            s_innactiveUsers.push_back(user);
                            }
                        }
                    }
                else if (msg->data.result != CURLE_OK)
                    {
                    //failCounter++;
                    char error[256];
                    sprintf(error, "curl error number: %d", (int)msg->data.result);
                    s_stats.InsertStats(user->m_currentOperation, false, user->m_downloadStart, Utf8String(error));
                    //std::cout << "curl not ok" << std::endl;
                    if(s_keepRunning)
                        {
                        if(user->m_retryCounter < 10)
                            {
                            user->m_currentOperation = OperationType::SPATIAL;
                            user->SelectExtents();
                            user->DoNext(this);
                            }
                        else
                            std::cout << "max retries reached, user giving up" << std::endl;
                        }
                    }

                curl_multi_remove_handle(m_pCurlHandle, msg->easy_handle);
                curl_easy_cleanup(msg->easy_handle);
                }
            else
                {
                //failCounter++;
                char *pClient;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &pClient);
                struct User *user = (struct User *)pClient;
                s_stats.InsertStats(user->m_currentOperation, false, user->m_downloadStart, "unhandled curl failure");
                //std::cout << "curl not ok" << std::endl;
                }
            }

        if (s_keepRunning && still_running == 0)
            {
            std::cout << "user pool empty, repopulating" << std::endl;
            Repopulate();
            }

        } while (still_running);
        
    }

void UserManager::Repopulate()
    {
    std::lock_guard<std::mutex> lock(innactiveUserMutex);
    size_t innactiveUserCount = s_innactiveUsers.size();

    for (size_t i = 0; i < innactiveUserCount; i++)
        {
        User* user = s_innactiveUsers.back();
        s_innactiveUsers.pop_back();
        user->DoNext(this);
        }
    }

void UserManager::SetupCurl(User* user, Utf8StringCR url, Utf8StringCP retString, FILE* fp, Utf8StringCR postFields)
    {
    CURL *pCurl = NULL;

    pCurl = curl_easy_init();
    if (pCurl)
        {
        struct curl_slist *headers = NULL;
        if (!postFields.empty())
            {
            headers = curl_slist_append(headers, "Accept: application/json");
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, "charsets: utf-8");
            curl_easy_setopt(pCurl, CURLOPT_POST, 1);
            curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, postFields.c_str());
            curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, postFields.length());
            }
        headers = curl_slist_append(headers, (*(user->m_token)).c_str());

        curl_easy_setopt(pCurl, CURLOPT_URL, url);

        curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 1);
        curl_easy_setopt(pCurl, CURLOPT_CAINFO, Utf8String(m_certPath));
        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(pCurl, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);
        if (nullptr != fp)
            {
            curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteData);
            curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);
            }
        else
            {
            curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, retString);
            }

        //curl_easy_setopt(pCurl, CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
        
        curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 0L);
        curl_easy_setopt(pCurl, CURLOPT_FTP_RESPONSE_TIMEOUT, 80L);

        curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 0L);

        curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(pCurl, CURLOPT_PRIVATE, user);

        user->m_downloadStart = std::time(nullptr);

        curl_multi_add_handle((CURLM*)m_pCurlHandle, pCurl);
        s_rps.AddRequest(user->m_currentOperation, user->m_downloadStart);
        }
    }
