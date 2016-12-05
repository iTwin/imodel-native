#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <thread>
#include <mutex>
#include <random>

#include <Bentley/BeTest.h>
#include <Bentley/BeFile.h>
#include <RealityPlatform/RealityConversionTools.h>
#include <RealityPlatform/RealityDataDownload.h>

#include "RealityContextTest.h"

#define MAX_NB_CONNECTIONS          25
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

        s_stats.PrintStats();
        }
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

void Stats::InsertStats(OperationType type, bool success, time_t time)
    {
    std::lock_guard<std::mutex> lock(statMutex);
    opStats[type]->Update(success, time);
    }

void Stats::PrintStats()
    {
    std::lock_guard<std::mutex> statlock(statMutex);
    std::lock_guard<std::mutex> rpslock(rpsMutex);
    time_t currentTime = std::time(nullptr);
    system("cls");

    std::cout << "Type      Success    Failure   minTime   maxTime   avgTime  requests/second" << std::endl;
    std::cout << "Spatial      " << opStats[OperationType::SPATIAL]->success << "        " << opStats[OperationType::SPATIAL]->failure << "         ";
    std::cout << opStats[OperationType::SPATIAL]->minTime << "         " << opStats[OperationType::SPATIAL]->maxTime << "        " ;
    std::cout << opStats[OperationType::SPATIAL]->avgTime << "         " << s_rps.GetRPS(OperationType::SPATIAL, currentTime) << std::endl;

    std::cout << "PackId       " << opStats[OperationType::PACKID]->success << "        " << opStats[OperationType::PACKID]->failure << "         ";
    std::cout << opStats[OperationType::PACKID]->minTime << "         " << opStats[OperationType::PACKID]->maxTime << "         ";
    std::cout << opStats[OperationType::PACKID]->avgTime << "          " << s_rps.GetRPS(OperationType::PACKID, currentTime) << std::endl;

    std::cout << "Package      " << opStats[OperationType::PACKFILE]->success << "        " << opStats[OperationType::PACKFILE]->failure << "         ";
    std::cout << opStats[OperationType::PACKFILE]->minTime << "         " << opStats[OperationType::PACKFILE]->maxTime << "         ";
    std::cout << opStats[OperationType::PACKFILE]->avgTime << "          " << s_rps.GetRPS(OperationType::PACKFILE, currentTime) << std::endl;
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
        m_bench->SetGeoParam(RealityPlatform::GeoCoordinationParams(filterPolygon));
    }

User::User(Utf8StringP token, std::default_random_engine* generator, std::uniform_real_distribution<double>* distribution):
    m_token(token), m_currentOperation(OperationType::SPATIAL), m_packageFile(nullptr), m_retryCounter(0),
    m_generator(generator), m_distribution(distribution)
    {
    SelectRegion();
    m_bench = RealityPlatform::ContextServicesWorkbench::Create(*token, RealityPlatform::GeoCoordinationParams());
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
    
    s_stats.InsertStats(OperationType::SPATIAL, retval, currentTime - m_downloadStart);

    return retval;
    }

bool User::ValidatePackageId()
    {
    time_t currentTime = std::time(nullptr);
    bool retval = true;
    Json::Value packageInfos(Json::objectValue);
    Json::Reader::Parse(*(m_bench->GetPackageIdPointer()), packageInfos);

    if (!packageInfos.isMember("changedInstance"))
        return false;

    m_bench->SetInstanceId(packageInfos["changedInstance"]["instanceAfterChange"]["instanceId"].asCString());

    if (retval)
        {
        m_currentOperation = OperationType::PACKFILE;
        m_retryCounter = 0;
        }

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

    s_stats.InsertStats(OperationType::PACKFILE, retval, currentTime - m_downloadStart);

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

Utf8String GetToken()
    {
    /*Utf8String retString = "";
    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);

    BeFileName testPath(exeDir);
    testPath.AppendToPath(L"TokenExtractor.exe");

    std::system(testPath.GetNameUtf8().c_str());

    HGLOBAL   hglb;
    LPTSTR    lptstr;

    if (!IsClipboardFormatAvailable(CF_TEXT))
        return "";
    if (!OpenClipboard(NULL))
        return "";

    hglb = GetClipboardData(CF_TEXT);
    if (hglb != NULL)
    {
        lptstr = (LPTSTR)GlobalLock(hglb);
        if (lptstr != NULL)
        {
            retString = Utf8String(lptstr);
            GlobalUnlock(hglb);
        }
    }
    CloseClipboard();

    return retString;*/
    return "Authorization: Token PHNhbWw6QXNzZXJ0aW9uIE1ham9yVmVyc2lvbj0iMSIgTWlub3JWZXJzaW9uPSIxIiBBc3NlcnRpb25JRD0iXzQ0N2IwYTY3LWU2NjMtNDRlNS05M2FhLWZhOTE2ZWJkZDgwNiIgSXNzdWVyPSJodHRwczovL3FhLWltcy5iZW50bGV5LmNvbS8iIElzc3VlSW5zdGFudD0iMjAxNi0xMi0wNVQyMDo1NTowMS45MjhaIiB4bWxuczpzYW1sPSJ1cm46b2FzaXM6bmFtZXM6dGM6U0FNTDoxLjA6YXNzZXJ0aW9uIj48c2FtbDpDb25kaXRpb25zIE5vdEJlZm9yZT0iMjAxNi0xMi0wNVQyMDo1NTowMS44MDNaIiBOb3RPbk9yQWZ0ZXI9IjIwMTYtMTItMDZUMDA6NTU6MDEuODAzWiI+PHNhbWw6QXVkaWVuY2VSZXN0cmljdGlvbkNvbmRpdGlvbj48c2FtbDpBdWRpZW5jZT5odHRwczovL3FhLXdhei1zZWFyY2guYmVudGxleS5jb20vPC9zYW1sOkF1ZGllbmNlPjwvc2FtbDpBdWRpZW5jZVJlc3RyaWN0aW9uQ29uZGl0aW9uPjwvc2FtbDpDb25kaXRpb25zPjxzYW1sOkF0dHJpYnV0ZVN0YXRlbWVudD48c2FtbDpTdWJqZWN0PjxzYW1sOk5hbWVJZGVudGlmaWVyPjhjMDNlN2FjLWRmMjgtNDhkOS1iNmFkLThjYjg2ZTQxNzUyMTwvc2FtbDpOYW1lSWRlbnRpZmllcj48c2FtbDpTdWJqZWN0Q29uZmlybWF0aW9uPjxzYW1sOkNvbmZpcm1hdGlvbk1ldGhvZD51cm46b2FzaXM6bmFtZXM6dGM6U0FNTDoxLjA6Y206aG9sZGVyLW9mLWtleTwvc2FtbDpDb25maXJtYXRpb25NZXRob2Q+PEtleUluZm8geG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvMDkveG1sZHNpZyMiPjx0cnVzdDpCaW5hcnlTZWNyZXQgeG1sbnM6dHJ1c3Q9Imh0dHA6Ly9kb2NzLm9hc2lzLW9wZW4ub3JnL3dzLXN4L3dzLXRydXN0LzIwMDUxMiI+MGdRakNYYTB2c3IyQ01obktJeThCSWZvUlRiRUw3bTF1K1h2Tjlsdk1haz08L3RydXN0OkJpbmFyeVNlY3JldD48L0tleUluZm8+PC9zYW1sOlN1YmplY3RDb25maXJtYXRpb24+PC9zYW1sOlN1YmplY3Q+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9Im5hbWUiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMueG1sc29hcC5vcmcvd3MvMjAwNS8wNS9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPmJjY191c2VyNUBtYWlsaW5hdG9yLmNvbTwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJnaXZlbm5hbWUiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMueG1sc29hcC5vcmcvd3MvMjAwNS8wNS9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPkVkd2FyZDwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJzdXJuYW1lIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLnhtbHNvYXAub3JnL3dzLzIwMDUvMDUvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT5FdmFuczwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJlbWFpbGFkZHJlc3MiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMueG1sc29hcC5vcmcvd3MvMjAwNS8wNS9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPmJjY191c2VyNUBtYWlsaW5hdG9yLmNvbTwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJyb2xlIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLm1pY3Jvc29mdC5jb20vd3MvMjAwOC8wNi9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPlNJVEVfQURNSU5JU1RSQVRPUjwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJzYXBidXBhIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT4xMDAzOTAzNjI4PC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9InVsdGltYXRlc2l0ZSIgQXR0cmlidXRlTmFtZXNwYWNlPSJodHRwOi8vc2NoZW1hcy5iZW50bGV5LmNvbS93cy8yMDExLzAzL2lkZW50aXR5L2NsYWltcyI+PHNhbWw6QXR0cmlidXRlVmFsdWU+MTAwMTM4MTg0MDwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJzYXBlbnRpdGxlbWVudCIgQXR0cmlidXRlTmFtZXNwYWNlPSJodHRwOi8vc2NoZW1hcy5iZW50bGV5LmNvbS93cy8yMDExLzAzL2lkZW50aXR5L2NsYWltcyI+PHNhbWw6QXR0cmlidXRlVmFsdWU+TkVXX0JFTlRMRVlfTEVBUk48L3NhbWw6QXR0cmlidXRlVmFsdWU+PHNhbWw6QXR0cmlidXRlVmFsdWU+RUxTX0ZPVVJfSFI8L3NhbWw6QXR0cmlidXRlVmFsdWU+PC9zYW1sOkF0dHJpYnV0ZT48c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0iZW50aXRsZW1lbnQiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPk5FV19CRU5UTEVZX0xFQVJOPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPkVMU19Gb3VyX0hvdXI8L3NhbWw6QXR0cmlidXRlVmFsdWU+PC9zYW1sOkF0dHJpYnV0ZT48c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0iY291bnRyeWlzbyIgQXR0cmlidXRlTmFtZXNwYWNlPSJodHRwOi8vc2NoZW1hcy5iZW50bGV5LmNvbS93cy8yMDExLzAzL2lkZW50aXR5L2NsYWltcyI+PHNhbWw6QXR0cmlidXRlVmFsdWU+VVM8L3NhbWw6QXR0cmlidXRlVmFsdWU+PC9zYW1sOkF0dHJpYnV0ZT48c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0ibGFuZ3VhZ2Vpc28iIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPkVOPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9ImlzbWFya2V0aW5ncHJvc3BlY3QiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPmZhbHNlPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9ImlzYmVudGxleWVtcGxveWVlIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT5mYWxzZTwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJiZWNvbW11bml0aWVzdXNlcm5hbWUiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPjhDMDNFN0FDLURGMjgtNDhEOS1CNkFELThDQjg2RTQxNzUyMTwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJiZWNvbW11bml0aWVzZW1haWxhZGRyZXNzIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT5iY2NfdXNlcjVAbWFpbGluYXRvci5jb208L3NhbWw6QXR0cmlidXRlVmFsdWU+PC9zYW1sOkF0dHJpYnV0ZT48c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0idXNlcmlkIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT44YzAzZTdhYy1kZjI4LTQ4ZDktYjZhZC04Y2I4NmU0MTc1MjE8L3NhbWw6QXR0cmlidXRlVmFsdWU+PC9zYW1sOkF0dHJpYnV0ZT48c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0ib3JnYW5pemF0aW9uIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT5QZW5uc3lsdmFuaWEgRE9UPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9Imhhc19zZWxlY3QiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPnRydWU8L3NhbWw6QXR0cmlidXRlVmFsdWU+PC9zYW1sOkF0dHJpYnV0ZT48c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0ib3JnYW5pemF0aW9uaWQiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPjFkZmY3ZDYxLWE5ZDktNGFhMC04YWZlLTJjYmVhNmE1M2Q0ZDwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJvcmdhbml6YXRpb25uYW1lIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT5QZW5uc3lsdmFuaWEgRE9UPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9InVsdGltYXRlaWQiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPjEwMDEzODE4NDA8L3NhbWw6QXR0cmlidXRlVmFsdWU+PC9zYW1sOkF0dHJpYnV0ZT48c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0idWx0aW1hdGVuYW1lIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT5QZW5uc3lsdmFuaWEgRE9UPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9InVsdGltYXRlcmVmZXJlbmNlaWQiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPmUwNmM4YWQ2LTMxMDctNDVjNS04MjVjLTM1OTlhOTFlNDk5Nzwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJ1c2FnZWNvdW50cnlpc28iIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPlVTPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PC9zYW1sOkF0dHJpYnV0ZVN0YXRlbWVudD48ZHM6U2lnbmF0dXJlIHhtbG5zOmRzPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwLzA5L3htbGRzaWcjIj48ZHM6U2lnbmVkSW5mbz48ZHM6Q2Fub25pY2FsaXphdGlvbk1ldGhvZCBBbGdvcml0aG09Imh0dHA6Ly93d3cudzMub3JnLzIwMDEvMTAveG1sLWV4Yy1jMTRuIyIgLz48ZHM6U2lnbmF0dXJlTWV0aG9kIEFsZ29yaXRobT0iaHR0cDovL3d3dy53My5vcmcvMjAwMS8wNC94bWxkc2lnLW1vcmUjcnNhLXNoYTI1NiIgLz48ZHM6UmVmZXJlbmNlIFVSST0iI180NDdiMGE2Ny1lNjYzLTQ0ZTUtOTNhYS1mYTkxNmViZGQ4MDYiPjxkczpUcmFuc2Zvcm1zPjxkczpUcmFuc2Zvcm0gQWxnb3JpdGhtPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwLzA5L3htbGRzaWcjZW52ZWxvcGVkLXNpZ25hdHVyZSIgLz48ZHM6VHJhbnNmb3JtIEFsZ29yaXRobT0iaHR0cDovL3d3dy53My5vcmcvMjAwMS8xMC94bWwtZXhjLWMxNG4jIiAvPjwvZHM6VHJhbnNmb3Jtcz48ZHM6RGlnZXN0TWV0aG9kIEFsZ29yaXRobT0iaHR0cDovL3d3dy53My5vcmcvMjAwMS8wNC94bWxlbmMjc2hhMjU2IiAvPjxkczpEaWdlc3RWYWx1ZT5sMmNGUzlpZkVpL0x0Vk1Ib0hVcHZmOHFWcC9WSVBiVEdKRENXMTNucW5VPTwvZHM6RGlnZXN0VmFsdWU+PC9kczpSZWZlcmVuY2U+PC9kczpTaWduZWRJbmZvPjxkczpTaWduYXR1cmVWYWx1ZT5xV0cwOURJRW1KUDdYUHN4a3pZazdqK3VDRW9yckpvQXBYRWZuWXk5Z3R4cEdMVGMyNWtLL0tGcUV5czdqekJscVlMUTBwdVR3OS9BcWNCaDdpRDZZdTV4UGFzWndUTEhmSmlqQ3pQYXBIZWVOOXVVU21IS1VTTVY5OTVQRTlqZWp4WEhaT05Ha1NqdmlBdWhkYTNtakFuMWVIaGF1aDhCRW5rYno0QWJtdEU9PC9kczpTaWduYXR1cmVWYWx1ZT48S2V5SW5mbyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC8wOS94bWxkc2lnIyI+PFg1MDlEYXRhPjxYNTA5Q2VydGlmaWNhdGU+TUlJRjhqQ0NCTnFnQXdJQkFnSUtHMW96SEFBQUFBZjVNREFOQmdrcWhraUc5dzBCQVFVRkFEQ0JwekVMTUFrR0ExVUVCaE1DVlZNeEV6QVJCZ29Ka2lhSmsvSXNaQUVaRmdOamIyMHhGekFWQmdvSmtpYUprL0lzWkFFWkZnZGlaVzUwYkdWNU1Rc3dDUVlEVlFRSUV3SlFRVEVPTUF3R0ExVUVCeE1GUlhoMGIyNHhIREFhQmdOVkJBb1RFMEpsYm5Sc1pYa2dVM2x6ZEdWdGN5QkpibU14Q3pBSkJnTlZCQXNUQWtsVU1TSXdJQVlEVlFRREV4bGpaWEowYVdacFkyRjBaWE14TG1KbGJuUnNaWGt1WTI5dE1CNFhEVEUxTVRBeE9URTRNemd5T1ZvWERURTNNVEF4T0RFNE16Z3lPVm93ZlRFTE1Ba0dBMVVFQmhNQ1ZWTXhDekFKQmdOVkJBZ1RBbEJCTVE0d0RBWURWUVFIRXdWRmVIUnZiakVjTUJvR0ExVUVDaE1UUW1WdWRHeGxlU0JUZVhOMFpXMXpJRWx1WXpFTE1Ba0dBMVVFQ3hNQ1NWUXhKakFrQmdOVkJBTVRIV2x0Y3kxMGIydGxiaTF6YVdkdWFXNW5MbUpsYm5Sc1pYa3VZMjl0TUlHZk1BMEdDU3FHU0liM0RRRUJBUVVBQTRHTkFEQ0JpUUtCZ1FDMnJtMDVObzdvd3VQR2JjcmIrSjc2M2g2YXB3NUxWVks2eXRrUXduckxjZTBCZ01XajE5NldvV3l4UGZraTEzazQ3RFZNbTNNVlVuV1FBaE9qMXV6UFFIeHJoVUM4SUVYTkNEcTh6ZXdKZXBMVW1MT0N2OVFMMnZNN21POGdsMWs0SkhUYlZyZXdZTXNQYU0rSzU1d2dVQmhrWGZMaGQ0UWhyUmNvZ3QxVFd3SURBUUFCbzRJQ3l6Q0NBc2N3Q3dZRFZSMFBCQVFEQWdXZ01CTUdBMVVkSlFRTU1Bb0dDQ3NHQVFVRkJ3TUJNSGdHQ1NxR1NJYjNEUUVKRHdSck1Ha3dEZ1lJS29aSWh2Y05Bd0lDQWdDQU1BNEdDQ3FHU0liM0RRTUVBZ0lBZ0RBTEJnbGdoa2dCWlFNRUFTb3dDd1lKWUlaSUFXVURCQUV0TUFzR0NXQ0dTQUZsQXdRQkFqQUxCZ2xnaGtnQlpRTUVBUVV3QndZRkt3NERBZ2N3Q2dZSUtvWklodmNOQXdjd0hRWURWUjBPQkJZRUZFVE10Ui9sWXFqTHd0MmZyZGdnbXMwRHlKTmNNQjhHQTFVZEl3UVlNQmFBRkhJUjd0NGdIS0RuSTdYL2xaSnIyTDUzNFR6Uk1JR2lCZ05WSFI4RWdab3dnWmN3Z1pTZ2daR2dnWTZHU1doMGRIQTZMeTlqWlhKMGFXWnBZMkYwWlhNeExtSmxiblJzWlhrdVkyOXRMME5sY25SRmJuSnZiR3d2WTJWeWRHbG1hV05oZEdWek1TNWlaVzUwYkdWNUxtTnZiUzVqY215R1FXaDBkSEE2THk5alpYSjBjbVZ1WlhkaGJERXVZbVZ1ZEd4bGVTNWpiMjB2WTNKc0wyTmxjblJwWm1sallYUmxjekV1WW1WdWRHeGxlUzVqYjIwdVkzSnNNSUlCSHdZSUt3WUJCUVVIQVFFRWdnRVJNSUlCRFRCdkJnZ3JCZ0VGQlFjd0FvWmphSFIwY0RvdkwyTmxjblJwWm1sallYUmxjekV1WW1WdWRHeGxlUzVqYjIwdlEyVnlkRVZ1Y205c2JDOWpaWEowYVdacFkyRjBaWE14TG1KbGJuUnNaWGt1WTI5dFgyTmxjblJwWm1sallYUmxjekV1WW1WdWRHeGxlUzVqYjIwdVkzSjBNR2NHQ0NzR0FRVUZCekFDaGx0b2RIUndPaTh2WTJWeWRISmxibVYzWVd3eExtSmxiblJzWlhrdVkyOXRMMk55YkM5alpYSjBhV1pwWTJGMFpYTXhMbUpsYm5Sc1pYa3VZMjl0WDJObGNuUnBabWxqWVhSbGN6RXVZbVZ1ZEd4bGVTNWpiMjB1WTNKME1ERUdDQ3NHQVFVRkJ6QUJoaVZvZEhSd09pOHZZMlZ5ZEdsbWFXTmhkR1Z6TVM1aVpXNTBiR1Y1TG1OdmJTOXZZM053TUNFR0NTc0dBUVFCZ2pjVUFnUVVIaElBVndCbEFHSUFVd0JsQUhJQWRnQmxBSEl3RFFZSktvWklodmNOQVFFRkJRQURnZ0VCQUxBRXNSTkxmazlyekZ5ZVQ0blNZVXJjbWhXRnh1dGVzT2FmeTFvcVdyTGxyODZib0xWdnN1Y3RiN1RFbVJsWDAvT3JJd1RkZHJ6WU5JM010UG5GMlJKWDlyU21YTWdwRy9qK3FwcDdSQXhzTWlIYTVTLzFmUGZHZFpSZG9PUFREcWZNNldBU2NteTVWU0JxVkc4YnNZbXJ2T1A3SWEyelJ4R09qckI2UGJNaFNtZU9tUDkxeDNXcWhkOXFMbE4vRUhHTUEzSVJzdGFTWDNvcmQwYmpGYk5MM0FBVmxzbkZKalBJWW5ZMkJQVS9OZmNhMVdVUWpQMXcvNXA4QWV2RXpPLyt2V1F3clk5cTYrKzNOM3oyTDV5MTl4aUJ2QnlIWDNOUkFnU0UzKytBMnJkTnNKY1c2WnlFZDlFbUF6NC96SWsxZHQ2WGliY3htc1hib3dFY0QrOD08L1g1MDlDZXJ0aWZpY2F0ZT48L1g1MDlEYXRhPjwvS2V5SW5mbz48L2RzOlNpZ25hdHVyZT48L3NhbWw6QXNzZXJ0aW9uPg==";
    }

int main(int argc, char* argv[])
    {
    Utf8String token = GetToken();

    UserManager wo = UserManager();
    wo.m_certPath = GetPemLocation();

    for(int i = 0; i < MAX_NB_CONNECTIONS; i++)
        {
            wo.users.push_back(new User(&token, &wo.m_generator, &wo.m_distribution));
        }

    std::thread dispatch (Dispatch, &wo);

    wo.Perform();
    s_keepRunning = false;
    dispatch.join();
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

void UserManager::Perform()
    {
    // we can optionally limit the total amount of connections this multi handle uses 
    curl_multi_setopt(m_pCurlHandle, CURLMOPT_MAXCONNECTS, MAX_NB_CONNECTIONS);

    for (int i = 0; i < min(MAX_NB_CONNECTIONS, (int)users.size()); ++i)
        {
        //SetupCurl(users[i]);
        users[i]->DoNext(this);
        }
    
    int still_running; /* keep number of running handles */
    int repeats = 0;

    int failCounter = 0;
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
                    if(!user->ValidatePrevious())
                        failCounter++;

                    int keepGoing = rand() % 100;
                    if((user->m_currentOperation == OperationType::PACKFILE) || keepGoing < 20)
                        user->DoNext(this);
                    else
                        {
                        std::lock_guard<std::mutex> lock(innactiveUserMutex);
                        s_innactiveUsers.push_back(user);
                        }
                    }
                else if (msg->data.result != CURLE_OK)
                    {
                    failCounter++;
                    //std::cout << "curl not ok" << std::endl;

                    if(user->m_retryCounter < 10)
                        {
                        user->m_currentOperation = OperationType::SPATIAL;
                        user->SelectExtents();
                        user->DoNext(this);
                        }
                    else
                        std::cout << "max retries reached, user giving up" << std::endl;
                    }

                curl_multi_remove_handle(m_pCurlHandle, msg->easy_handle);
                curl_easy_cleanup(msg->easy_handle);
                }
            else
                {
                failCounter++;
                //std::cout << "curl not ok" << std::endl;
                }
            }

        if (still_running == 0)
            {
            std::cout << "user pool empty, repopulating" << std::endl;
            Repopulate();
            }

        } while (failCounter < 20);
        std::cout << "maximum allowed failures achieved, exiting" << std::endl;
        getch();
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
