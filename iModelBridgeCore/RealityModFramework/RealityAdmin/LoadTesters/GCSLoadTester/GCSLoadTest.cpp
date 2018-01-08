#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <thread>
#include <mutex>
#include <random>
#include <fstream>

#include <Bentley/BeTest.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeTimeUtilities.h>
#include <RealityPlatformTools/RealityConversionTools.h>
#include <RealityPlatformTools/RealityDataDownload.h>
#include <CCApi/CCPublic.h>

#include "GCSLoadTest.h"

#define M_PI                        3.1415926f

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

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

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//* request per second class
//* for each operation type, keep a cout of how many requests were sent at any given second
//+---------------+---------------+---------------+---------------+---------------+------*/
GCSRPS::GCSRPS() : RPS()
    {
    requestLog.Insert(OperationType::SPATIAL, bmap<int64_t, int>());
    requestLog.Insert(OperationType::PACKID, bmap<int64_t, int>());
    requestLog.Insert(OperationType::PACKFILE, bmap<int64_t, int>());
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//* for each operation type, tracks success and failures, as well as response time
//+---------------+---------------+---------------+---------------+---------------+------*/
GCSStats::GCSStats(GCSRPS* rps) : Stats(rps)
    {   
    opStats = bmap<int, Stat*>();
    opStats.Insert(OperationType::SPATIAL, new Stat());
    opStats.Insert(OperationType::PACKID, new Stat());
    opStats.Insert(OperationType::PACKFILE, new Stat());

    errors = bmap<int, bvector<Utf8String>>();
    errors.Insert(OperationType::SPATIAL, bvector<Utf8String>());
    errors.Insert(OperationType::PACKID, bvector<Utf8String>());
    errors.Insert(OperationType::PACKFILE, bvector<Utf8String>());
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
void GCSStats::PrintStatsBody(int64_t currentTime, int& totalSuccess, int& totalFailure)
    {
    system("cls");

    std::cout << "Type                    Success    Failure   minTime   maxTime   avgTime  requests/second" << std::endl;

    std::cout << Utf8PrintfString("Spatial               %6d %10d %9d %10d %9d        %f", opStats[OperationType::SPATIAL]->success, opStats[OperationType::SPATIAL]->failure, (int)opStats[OperationType::SPATIAL]->minTime, (int)opStats[OperationType::SPATIAL]->maxTime, (int)opStats[OperationType::SPATIAL]->avgTime, m_rps->GetRPS(OperationType::SPATIAL, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("PackId                %6d %10d %9d %10d %9d        %f", opStats[OperationType::PACKID]->success, opStats[OperationType::PACKID]->failure, (int)opStats[OperationType::PACKID]->minTime, (int)opStats[OperationType::PACKID]->maxTime, (int)opStats[OperationType::PACKID]->avgTime, m_rps->GetRPS(OperationType::PACKID, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Package               %6d %10d %9d %10d %9d        %f", opStats[OperationType::PACKFILE]->success, opStats[OperationType::PACKFILE]->failure, (int)opStats[OperationType::PACKFILE]->minTime, (int)opStats[OperationType::PACKFILE]->maxTime, (int)opStats[OperationType::PACKFILE]->avgTime, m_rps->GetRPS(OperationType::PACKFILE, currentTime)) << std::endl;
    std::cout << std::endl;

    totalSuccess = opStats[OperationType::SPATIAL]->success + opStats[OperationType::PACKID]->success +
        opStats[OperationType::PACKFILE]->success;

    totalFailure = opStats[OperationType::SPATIAL]->failure + opStats[OperationType::PACKID]->failure +
        opStats[OperationType::PACKFILE]->failure;

    std::cout << Utf8PrintfString("Total                 %6d %10d", totalSuccess, totalFailure) << std::endl;
    std::cout << Utf8PrintfString("Total Requests        %6d", totalSuccess + totalFailure) << std::endl;

    std::cout << std::endl;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
void GCSStats::WriteToFileBody(int userCount, Utf8String path, std::ofstream& file)
    {
    file << Utf8PrintfString("Spatial               %6d %10d %9d %10d %9d", opStats[OperationType::SPATIAL]->success, opStats[OperationType::SPATIAL]->failure, (int)opStats[OperationType::SPATIAL]->minTime, (int)opStats[OperationType::SPATIAL]->maxTime, (int)opStats[OperationType::SPATIAL]->avgTime) << std::endl;
    file << Utf8PrintfString("PackId                %6d %10d %9d %10d %9d", opStats[OperationType::PACKID]->success, opStats[OperationType::PACKID]->failure, (int)opStats[OperationType::PACKID]->minTime, (int)opStats[OperationType::PACKID]->maxTime, (int)opStats[OperationType::PACKID]->avgTime) << std::endl;
    file << Utf8PrintfString("PackFile              %6d %10d %9d %10d %9d", opStats[OperationType::PACKFILE]->success, opStats[OperationType::PACKFILE]->failure, (int)opStats[OperationType::PACKFILE]->minTime, (int)opStats[OperationType::PACKFILE]->maxTime, (int)opStats[OperationType::PACKFILE]->avgTime) << std::endl;
    
    file << std::endl << std::endl << "op list:" << std::endl;

    for (Utf8String op : opLog)
        file << op << std::endl;

    file << std::endl << std::endl << "error list:" << std::endl;

    file << "Spatial Request:" << std::endl;
    for (Utf8String error : errors[OperationType::SPATIAL])
        file << error << std::endl;

    file << "Get Package Id:" << std::endl;
    for (Utf8String error : errors[OperationType::PACKID])
        file << error << std::endl;

    file << "Download Package File:" << std::endl;
    for (Utf8String error : errors[OperationType::PACKFILE])
        file << error << std::endl;

    file.close();
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
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

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
void GCSUser::SelectRegion()
    {
    m_region = worldRegions[rand() % worldRegions.size()];
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
float GCSUser::Degree2Radians(float degree)
    {
    return (degree / 180.0f) * M_PI;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
float GCSUser::Radians2Degree(float radius)
    {
    return (radius / M_PI) * 180.0f;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//* randomly selects extents within the selected region
//+---------------+---------------+---------------+---------------+---------------+------*/
void GCSUser::SelectExtents()
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

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
GCSUser::GCSUser(Utf8StringP token, std::default_random_engine* generator, std::uniform_real_distribution<double>* distribution, int id, Stats* stats, RealityPlatform::CONNECTServerType serverType) : 
    User(id, stats), m_token(token), m_packageFile(nullptr), m_retryCounter(0),
    m_generator(generator), m_distribution(distribution), m_serverType(serverType)
    { 
    m_wrappedUp = true;
    m_currentOperation = OperationType::SPATIAL;
    SelectRegion();
    m_bench = RealityPlatform::ContextServicesWorkbench::Create("", RealityPlatform::GeoCoordinationParams());
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//* prepares next request, based on most recent action
//* can be called blindly from outside
//+---------------+---------------+---------------+---------------+---------------+------*/
bool GCSUser::DoNextBody(UserManager* owner)
    {
    CURL* pCurl = nullptr;
    switch(m_currentOperation)
        {
        case OperationType::SPATIAL:
            {
            m_bench->Init();
            SelectExtents();
            pCurl = SetupCurl(m_bench->CreateSpatialEntityWithDetailsViewUrl(), m_bench->GetSpatialEntityWithDetailsJsonPointer());
            m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("User %d : Spatial for %s", m_userId, m_bench->CreateSpatialEntityWithDetailsViewUrl()));
            break;
            }
        case OperationType::PACKID:
            {
            m_packageParameters = m_bench->GetPackageParameters(m_selectedIds);
            pCurl = SetupCurl(m_bench->GetPackageIdUrl(), m_bench->GetPackageIdPointer(), nullptr, m_packageParameters);
            m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("User %d : Package Id", m_userId));
            break;
            }
        case OperationType::PACKFILE:
            {
            m_packageFile = m_bench->OpenPackageFile();
            if(!m_packageFile) //cannot open file on disc, local error not server error (not logged as an error)
                {
                std::cout << "open file failure, aborting user" << std::endl;
                return true;
                }
            pCurl = SetupCurl(m_bench->GetPackageFileUrl(), nullptr, m_packageFile);
            m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("User %d : Package File for %s", m_userId, m_bench->GetPackageFileUrl()));
            break;
            }
        }

    if (pCurl != nullptr)
        {
        owner->SetupCurl(pCurl, this);
        return false;
        }
    return true;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//* randomly select ids from spatial query
//+---------------+---------------+---------------+---------------+---------------+------*/
void GCSUser::SampleIds(Json::Value regionItems)
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

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//* ensures that the json received has the proper format 
//+---------------+---------------+---------------+---------------+---------------+------*/
bool GCSUser::ValidateSpatial(int activeUsers)
    {
    bool retval = true;
    Json::Value regionItems(Json::objectValue);
    if (!Json::Reader::Parse(m_bench->GetSpatialEntityWithDetailsJson(), regionItems) || (!regionItems.isMember("errorMessage") && !regionItems.isMember("instances")))
        {
        retval = false;
        }
    else if (regionItems.isMember("errorMessage"))
        {
        retval = false;
        }

    m_correspondance.response.body = *(m_bench->GetSpatialEntityWithDetailsJsonPointer());
    m_stats->InsertStats(this, retval, activeUsers);

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

    return retval;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//* ensures json response contains package id
//+---------------+---------------+---------------+---------------+---------------+------*/
bool GCSUser::ValidatePackageId(int activeUsers)
    {
    bool retval = true;
    Json::Value packageInfos(Json::objectValue);
    Json::Reader::Parse(*(m_bench->GetPackageIdPointer()), packageInfos);

    m_correspondance.response.body = *(m_bench->GetPackageIdPointer());
    if (!packageInfos.isMember("changedInstance"))
        {
        retval = false;
        m_stats->InsertStats(this, retval, activeUsers);
        return retval;
        }

    m_bench->SetInstanceId(packageInfos["changedInstance"]["instanceAfterChange"]["instanceId"].asCString());

    m_stats->InsertStats(this, retval, activeUsers);

    m_currentOperation = OperationType::PACKFILE;
    m_retryCounter = 0;

    return retval;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//* ensures that the file is on the filesystem
//+---------------+---------------+---------------+---------------+---------------+------*/
bool GCSUser::ValidatePacakgeFile(int activeUsers)
    {
    bool retval = true;

    fclose(m_packageFile);

    BeFile file;
    file.Open(m_bench->GetPackageFileName().c_str(), BeFileAccess::Read);
    uint64_t packageSize;
    file.GetSize(packageSize);
    if(packageSize < 1)
        retval = false;
    file.Close();

    m_correspondance.response.body = "PackageFile: package is empty";
    m_stats->InsertStats(this, retval, activeUsers);

    if (retval)
        {
        m_bench->SetPackageDownloaded(true);

        m_currentOperation = OperationType::SPATIAL;
        BeFileName::BeDeleteFile(m_bench->GetPackageFileName().c_str()); //clean up and remove the package file
        m_retryCounter = 0;
        }

    return retval;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//* automatically call the proper validation function for the last executed operation
//+---------------+---------------+---------------+---------------+---------------+------*/
void GCSUser::ValidatePrevious(int activeUsers)
    {
    bool retval = true;
    switch (m_currentOperation)
        {
        case OperationType::SPATIAL:
            retval = ValidateSpatial(activeUsers);
            break;

        case OperationType::PACKID:
            retval = ValidatePackageId(activeUsers);
            break;

        case OperationType::PACKFILE:
            retval = ValidatePacakgeFile(activeUsers);
            break;

        case OperationType::FILES:
            if (retval)
                m_currentOperation = OperationType::SPATIAL;
            break;
        }
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
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

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
GCSUser::~GCSUser()
    {
    delete m_bench;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
int main(int argc, char* argv[])
    {
    LoadTester tester = LoadTester();
    GCSRPS rps = GCSRPS();
    GCSStats stats = GCSStats(&rps);

    LoadTester::SetupStaticClasses(&stats, &rps);
    if (tester.Main(argc, argv))
        return 0;

    SetConsoleTitle("Reality Context Test");
     
    //use CCApi to get a token from the CONNECTION Client
    CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
    CallStatus status = APIERR_SUCCESS;

    bool installed;
    status = CCApi_IsInstalled(api, &installed);
    if (!installed)
        {
        cout << "Client not installed, or COM not registered" << endl;
        CCApi_FreeApi(api);
        getch();
        return -1;
        }
    bool running = false;
    status = CCApi_IsRunning(api, &running);
    if (status != APIERR_SUCCESS)
        {
        wprintf(L"IsRunning failed with error code %d\n", status);
        CCApi_FreeApi(api);
        getch();
        return -1;
        }
    if (!running)
        {
        cout << "Client not running" << endl;
        CCApi_FreeApi(api);
        getch();
        return -1;
        }
    bool loggedIn = false;
    status = CCApi_IsLoggedIn(api, &loggedIn);
    if (status != APIERR_SUCCESS)
        {
        wprintf(L"IsLoggedIn failed with error code %d\n", status);
        CCApi_FreeApi(api);
        getch();
        return -1;
        }
    if (!loggedIn)
        {
        cout << "Client not logged in" << endl;
        CCApi_FreeApi(api);
        getch();
        return -1;
        }
    bool acceptedEula = false;
    status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
    if (status != APIERR_SUCCESS)
        {
        wprintf(L"CCApi_HasUserAcceptedEULA failed with error code %d\n", status);
        CCApi_FreeApi(api);
        getch();
        return -1;
        }
    if (!acceptedEula)
        {
        cout << "Client has not accepted the EULA" << endl;
        CCApi_FreeApi(api);
        getch();
        return -1;
        }
    bool sessionActive = false;
    status = CCApi_IsUserSessionActive(api, &sessionActive);
    if (status != APIERR_SUCCESS)
        {
        wprintf(L"CCApi_IsUserSessionActive failed with error code %d\n", status);
        CCApi_FreeApi(api);
        getch();
        return -1;
        }
    if (!sessionActive)
        {
        cout << "Session not active" << endl;
        CCApi_FreeApi(api);
        getch();
        return -1;
        }

    LPCWSTR relyingParty = L"https://connect-wsg20.bentley.com";//;L"https:://qa-ims.bentley.com"
    UINT32 maxTokenLength = 16384;
    LPWSTR lpwstrToken = new WCHAR[maxTokenLength];

    status = CCApi_GetSerializedDelegateSecurityToken(api, relyingParty, lpwstrToken, maxTokenLength);
    if (status != APIERR_SUCCESS)
        {
        wprintf(L"CCApi_GetSerializedDelegateSecurityToken failed with error code %d\n", status);
        CCApi_FreeApi(api);
        getch();
        return -1;
        }

    char* charToken = new char[maxTokenLength];
    wcstombs(charToken, lpwstrToken, maxTokenLength);

    Utf8String token = "Authorization: Token ";//PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0idXRmLTE2Ij8+PHNhbWw6QXNzZXJ0aW9uIE1ham9yVmVyc2lvbj0iMSIgTWlub3JWZXJzaW9uPSIxIiBBc3NlcnRpb25JRD0iX2YzMTU1M2UxLTg5MWItNDM0OS05MTk5LWFmYWE0OGFkMGM1YyIgSXNzdWVyPSJodHRwczovL3FhLWltcy5iZW50bGV5LmNvbS8iIElzc3VlSW5zdGFudD0iMjAxNi0xMi0xM1QxNTowOTowMC4xMTFaIiB4bWxuczpzYW1sPSJ1cm46b2FzaXM6bmFtZXM6dGM6U0FNTDoxLjA6YXNzZXJ0aW9uIj48c2FtbDpDb25kaXRpb25zIE5vdEJlZm9yZT0iMjAxNi0xMi0xM1QxNTowOTowMC4wNjRaIiBOb3RPbk9yQWZ0ZXI9IjIwMTYtMTItMTNUMTk6MDk6MDAuMDY0WiI+PHNhbWw6QXVkaWVuY2VSZXN0cmljdGlvbkNvbmRpdGlvbj48c2FtbDpBdWRpZW5jZT5odHRwczovL2Nvbm5lY3Qtd3NnMjAuYmVudGxleS5jb208L3NhbWw6QXVkaWVuY2U+PC9zYW1sOkF1ZGllbmNlUmVzdHJpY3Rpb25Db25kaXRpb24+PC9zYW1sOkNvbmRpdGlvbnM+PHNhbWw6QXR0cmlidXRlU3RhdGVtZW50PjxzYW1sOlN1YmplY3Q+PHNhbWw6TmFtZUlkZW50aWZpZXI+OTI1NzY4MTctN2YwMS00N2U2LWIzNTktMjM4ZGQ0ZGJjMjgwPC9zYW1sOk5hbWVJZGVudGlmaWVyPjxzYW1sOlN1YmplY3RDb25maXJtYXRpb24+PHNhbWw6Q29uZmlybWF0aW9uTWV0aG9kPnVybjpvYXNpczpuYW1lczp0YzpTQU1MOjEuMDpjbTpob2xkZXItb2Yta2V5PC9zYW1sOkNvbmZpcm1hdGlvbk1ldGhvZD48S2V5SW5mbyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC8wOS94bWxkc2lnIyI+PHRydXN0OkJpbmFyeVNlY3JldCB4bWxuczp0cnVzdD0iaHR0cDovL2RvY3Mub2FzaXMtb3Blbi5vcmcvd3Mtc3gvd3MtdHJ1c3QvMjAwNTEyIj5QQnNMVW5XLzNDSzc5UmpmRm4ydnRZbURSamhiT20yTis0WHVVQVl6N0Z3PTwvdHJ1c3Q6QmluYXJ5U2VjcmV0PjwvS2V5SW5mbz48L3NhbWw6U3ViamVjdENvbmZpcm1hdGlvbj48L3NhbWw6U3ViamVjdD48c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0ibmFtZSIgQXR0cmlidXRlTmFtZXNwYWNlPSJodHRwOi8vc2NoZW1hcy54bWxzb2FwLm9yZy93cy8yMDA1LzA1L2lkZW50aXR5L2NsYWltcyI+PHNhbWw6QXR0cmlidXRlVmFsdWU+ZnJhbmNpcy5ib2lseUBiZW50bGV5LmNvbTwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJnaXZlbm5hbWUiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMueG1sc29hcC5vcmcvd3MvMjAwNS8wNS9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPkZyYW5jaXM8L3NhbWw6QXR0cmlidXRlVmFsdWU+PC9zYW1sOkF0dHJpYnV0ZT48c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0ic3VybmFtZSIgQXR0cmlidXRlTmFtZXNwYWNlPSJodHRwOi8vc2NoZW1hcy54bWxzb2FwLm9yZy93cy8yMDA1LzA1L2lkZW50aXR5L2NsYWltcyI+PHNhbWw6QXR0cmlidXRlVmFsdWU+Qm9pbHk8L3NhbWw6QXR0cmlidXRlVmFsdWU+PC9zYW1sOkF0dHJpYnV0ZT48c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0iZW1haWxhZGRyZXNzIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLnhtbHNvYXAub3JnL3dzLzIwMDUvMDUvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT5mcmFuY2lzLmJvaWx5QGJlbnRsZXkuY29tPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9InJvbGUiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMubWljcm9zb2Z0LmNvbS93cy8yMDA4LzA2L2lkZW50aXR5L2NsYWltcyI+PHNhbWw6QXR0cmlidXRlVmFsdWU+QkVOVExFWV9FTVBMT1lFRTwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJzYXBidXBhIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT4wMDA4MDEzOTkxPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9InNpdGUiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPjQwMjI3MDY8L3NhbWw6QXR0cmlidXRlVmFsdWU+PHNhbWw6QXR0cmlidXRlVmFsdWU+MTAwNDEzMTU1Mjwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJ1bHRpbWF0ZXNpdGUiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPjEwMDEzODkxMTc8L3NhbWw6QXR0cmlidXRlVmFsdWU+PC9zYW1sOkF0dHJpYnV0ZT48c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0ic2FwZW50aXRsZW1lbnQiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPklOVEVSTkFMPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPlNFTEVDVF8yMDA2PC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9ImVudGl0bGVtZW50IiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT5CRU5UTEVZX0VNUExPWUVFPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPlNFTEVDVF8yMDA2PC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9ImNvdW50cnlpc28iIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPkNBPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9Imxhbmd1YWdlaXNvIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT5FTjwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJpc21hcmtldGluZ3Byb3NwZWN0IiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT5mYWxzZTwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJpc2JlbnRsZXllbXBsb3llZSIgQXR0cmlidXRlTmFtZXNwYWNlPSJodHRwOi8vc2NoZW1hcy5iZW50bGV5LmNvbS93cy8yMDExLzAzL2lkZW50aXR5L2NsYWltcyI+PHNhbWw6QXR0cmlidXRlVmFsdWU+dHJ1ZTwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJiZWNvbW11bml0aWVzdXNlcm5hbWUiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPjkyNTc2ODE3LTdGMDEtNDdFNi1CMzU5LTIzOERENERCQzI4MDwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJiZWNvbW11bml0aWVzZW1haWxhZGRyZXNzIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT5mcmFuY2lzLmJvaWx5QGJlbnRsZXkuY29tPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9InVzZXJpZCIgQXR0cmlidXRlTmFtZXNwYWNlPSJodHRwOi8vc2NoZW1hcy5iZW50bGV5LmNvbS93cy8yMDExLzAzL2lkZW50aXR5L2NsYWltcyI+PHNhbWw6QXR0cmlidXRlVmFsdWU+OTI1NzY4MTctN2YwMS00N2U2LWIzNTktMjM4ZGQ0ZGJjMjgwPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9Im9yZ2FuaXphdGlvbiIgQXR0cmlidXRlTmFtZXNwYWNlPSJodHRwOi8vc2NoZW1hcy5iZW50bGV5LmNvbS93cy8yMDExLzAzL2lkZW50aXR5L2NsYWltcyI+PHNhbWw6QXR0cmlidXRlVmFsdWU+QmVudGxleSBTeXN0ZW1zIEluYzwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJoYXNfc2VsZWN0IiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT50cnVlPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9Im9yZ2FuaXphdGlvbmlkIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT5lODJhNTg0Yi05ZmFlLTQwOWYtOTU4MS1mZDE1NGY3YjllZjk8L3NhbWw6QXR0cmlidXRlVmFsdWU+PC9zYW1sOkF0dHJpYnV0ZT48c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0ib3JnYW5pemF0aW9ubmFtZSIgQXR0cmlidXRlTmFtZXNwYWNlPSJodHRwOi8vc2NoZW1hcy5iZW50bGV5LmNvbS93cy8yMDExLzAzL2lkZW50aXR5L2NsYWltcyI+PHNhbWw6QXR0cmlidXRlVmFsdWU+QmVudGxleSBTeXN0ZW1zIEluYzwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJ1bHRpbWF0ZWlkIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT43MmFkYWQzMC1jMDdjLTQ2NWQtYTFmZS0yZjJkZmFjOTUwYTQ8L3NhbWw6QXR0cmlidXRlVmFsdWU+PC9zYW1sOkF0dHJpYnV0ZT48c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0idWx0aW1hdGVuYW1lIiBBdHRyaWJ1dGVOYW1lc3BhY2U9Imh0dHA6Ly9zY2hlbWFzLmJlbnRsZXkuY29tL3dzLzIwMTEvMDMvaWRlbnRpdHkvY2xhaW1zIj48c2FtbDpBdHRyaWJ1dGVWYWx1ZT5CZW50bGV5IFN5c3RlbXMgSW5jPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9InVsdGltYXRlcmVmZXJlbmNlaWQiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPjcyYWRhZDMwLWMwN2MtNDY1ZC1hMWZlLTJmMmRmYWM5NTBhNDwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjxzYW1sOkF0dHJpYnV0ZSBBdHRyaWJ1dGVOYW1lPSJ1c2FnZWNvdW50cnlpc28iIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPkNBPC9zYW1sOkF0dHJpYnV0ZVZhbHVlPjwvc2FtbDpBdHRyaWJ1dGU+PHNhbWw6QXR0cmlidXRlIEF0dHJpYnV0ZU5hbWU9InVsdGltYXRlc2FwaWQiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMuYmVudGxleS5jb20vd3MvMjAxMS8wMy9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPjEwMDEzODkxMTc8L3NhbWw6QXR0cmlidXRlVmFsdWU+PC9zYW1sOkF0dHJpYnV0ZT48c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0iYWN0b3IiIEF0dHJpYnV0ZU5hbWVzcGFjZT0iaHR0cDovL3NjaGVtYXMueG1sc29hcC5vcmcvd3MvMjAwOS8wOS9pZGVudGl0eS9jbGFpbXMiPjxzYW1sOkF0dHJpYnV0ZVZhbHVlPiZsdDtBY3RvciZndDsmbHQ7c2FtbDpBdHRyaWJ1dGUgQXR0cmlidXRlTmFtZT0ibmFtZSIgQXR0cmlidXRlTmFtZXNwYWNlPSJodHRwOi8vc2NoZW1hcy54bWxzb2FwLm9yZy93cy8yMDA1LzA1L2lkZW50aXR5L2NsYWltcyIgeG1sbnM6c2FtbD0idXJuOm9hc2lzOm5hbWVzOnRjOlNBTUw6MS4wOmFzc2VydGlvbiImZ3Q7Jmx0O3NhbWw6QXR0cmlidXRlVmFsdWUmZ3Q7Q049aW1zLXRva2VuLXNpZ25pbmcuYmVudGxleS5jb20sIE9VPUlULCBPPUJlbnRsZXkgU3lzdGVtcyBJbmMsIEw9RXh0b24sIFM9UEEsIEM9VVMmbHQ7L3NhbWw6QXR0cmlidXRlVmFsdWUmZ3Q7Jmx0Oy9zYW1sOkF0dHJpYnV0ZSZndDsmbHQ7L0FjdG9yJmd0Ozwvc2FtbDpBdHRyaWJ1dGVWYWx1ZT48L3NhbWw6QXR0cmlidXRlPjwvc2FtbDpBdHRyaWJ1dGVTdGF0ZW1lbnQ+PGRzOlNpZ25hdHVyZSB4bWxuczpkcz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC8wOS94bWxkc2lnIyI+PGRzOlNpZ25lZEluZm8+PGRzOkNhbm9uaWNhbGl6YXRpb25NZXRob2QgQWxnb3JpdGhtPSJodHRwOi8vd3d3LnczLm9yZy8yMDAxLzEwL3htbC1leGMtYzE0biMiIC8+PGRzOlNpZ25hdHVyZU1ldGhvZCBBbGdvcml0aG09Imh0dHA6Ly93d3cudzMub3JnLzIwMDEvMDQveG1sZHNpZy1tb3JlI3JzYS1zaGEyNTYiIC8+PGRzOlJlZmVyZW5jZSBVUkk9IiNfZjMxNTUzZTEtODkxYi00MzQ5LTkxOTktYWZhYTQ4YWQwYzVjIj48ZHM6VHJhbnNmb3Jtcz48ZHM6VHJhbnNmb3JtIEFsZ29yaXRobT0iaHR0cDovL3d3dy53My5vcmcvMjAwMC8wOS94bWxkc2lnI2VudmVsb3BlZC1zaWduYXR1cmUiIC8+PGRzOlRyYW5zZm9ybSBBbGdvcml0aG09Imh0dHA6Ly93d3cudzMub3JnLzIwMDEvMTAveG1sLWV4Yy1jMTRuIyIgLz48L2RzOlRyYW5zZm9ybXM+PGRzOkRpZ2VzdE1ldGhvZCBBbGdvcml0aG09Imh0dHA6Ly93d3cudzMub3JnLzIwMDEvMDQveG1sZW5jI3NoYTI1NiIgLz48ZHM6RGlnZXN0VmFsdWU+Q1ZGc3hGU0R1bEMyOHNacThVY1RTUW43UFY2dFY2aWFXVWU3WCtlOFoybz08L2RzOkRpZ2VzdFZhbHVlPjwvZHM6UmVmZXJlbmNlPjwvZHM6U2lnbmVkSW5mbz48ZHM6U2lnbmF0dXJlVmFsdWU+Z0ZUeUtmNTB5dDZ3Mnd0NE1Wci9xb3h4UW9pdDA0TUdieGt1aHNoTlpMcVVlQ28zUlhLR1ZaenluTTRlcjZXY1RncFVQWVlsR0h5SXAvY3B6cndLUWU4LzVoN0EvWVIzVkF6UzM0eFRUMEgvMzllcE04aWRtcG1XV1h5N3o1YzZBK3owOHNBcHFZZVJ2TXpMbFloRjd5c1gvZ2xxR0RwaWtRWXl0WlI4bStNPTwvZHM6U2lnbmF0dXJlVmFsdWU+PEtleUluZm8geG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvMDkveG1sZHNpZyMiPjxYNTA5RGF0YT48WDUwOUNlcnRpZmljYXRlPk1JSUY4akNDQk5xZ0F3SUJBZ0lLRzFvekhBQUFBQWY1TURBTkJna3Foa2lHOXcwQkFRVUZBRENCcHpFTE1Ba0dBMVVFQmhNQ1ZWTXhFekFSQmdvSmtpYUprL0lzWkFFWkZnTmpiMjB4RnpBVkJnb0praWFKay9Jc1pBRVpGZ2RpWlc1MGJHVjVNUXN3Q1FZRFZRUUlFd0pRUVRFT01Bd0dBMVVFQnhNRlJYaDBiMjR4SERBYUJnTlZCQW9URTBKbGJuUnNaWGtnVTNsemRHVnRjeUJKYm1NeEN6QUpCZ05WQkFzVEFrbFVNU0l3SUFZRFZRUURFeGxqWlhKMGFXWnBZMkYwWlhNeExtSmxiblJzWlhrdVkyOXRNQjRYRFRFMU1UQXhPVEU0TXpneU9Wb1hEVEUzTVRBeE9ERTRNemd5T1Zvd2ZURUxNQWtHQTFVRUJoTUNWVk14Q3pBSkJnTlZCQWdUQWxCQk1RNHdEQVlEVlFRSEV3VkZlSFJ2YmpFY01Cb0dBMVVFQ2hNVFFtVnVkR3hsZVNCVGVYTjBaVzF6SUVsdVl6RUxNQWtHQTFVRUN4TUNTVlF4SmpBa0JnTlZCQU1USFdsdGN5MTBiMnRsYmkxemFXZHVhVzVuTG1KbGJuUnNaWGt1WTI5dE1JR2ZNQTBHQ1NxR1NJYjNEUUVCQVFVQUE0R05BRENCaVFLQmdRQzJybTA1Tm83b3d1UEdiY3JiK0o3NjNoNmFwdzVMVlZLNnl0a1F3bnJMY2UwQmdNV2oxOTZXb1d5eFBma2kxM2s0N0RWTW0zTVZVbldRQWhPajF1elBRSHhyaFVDOElFWE5DRHE4emV3SmVwTFVtTE9DdjlRTDJ2TTdtTzhnbDFrNEpIVGJWcmV3WU1zUGFNK0s1NXdnVUJoa1hmTGhkNFFoclJjb2d0MVRXd0lEQVFBQm80SUN5ekNDQXNjd0N3WURWUjBQQkFRREFnV2dNQk1HQTFVZEpRUU1NQW9HQ0NzR0FRVUZCd01CTUhnR0NTcUdTSWIzRFFFSkR3UnJNR2t3RGdZSUtvWklodmNOQXdJQ0FnQ0FNQTRHQ0NxR1NJYjNEUU1FQWdJQWdEQUxCZ2xnaGtnQlpRTUVBU293Q3dZSllJWklBV1VEQkFFdE1Bc0dDV0NHU0FGbEF3UUJBakFMQmdsZ2hrZ0JaUU1FQVFVd0J3WUZLdzREQWdjd0NnWUlLb1pJaHZjTkF3Y3dIUVlEVlIwT0JCWUVGRVRNdFIvbFlxakx3dDJmcmRnZ21zMER5Sk5jTUI4R0ExVWRJd1FZTUJhQUZISVI3dDRnSEtEbkk3WC9sWkpyMkw1MzRUelJNSUdpQmdOVkhSOEVnWm93Z1pjd2daU2dnWkdnZ1k2R1NXaDBkSEE2THk5alpYSjBhV1pwWTJGMFpYTXhMbUpsYm5Sc1pYa3VZMjl0TDBObGNuUkZibkp2Ykd3dlkyVnlkR2xtYVdOaGRHVnpNUzVpWlc1MGJHVjVMbU52YlM1amNteUdRV2gwZEhBNkx5OWpaWEowY21WdVpYZGhiREV1WW1WdWRHeGxlUzVqYjIwdlkzSnNMMk5sY25ScFptbGpZWFJsY3pFdVltVnVkR3hsZVM1amIyMHVZM0pzTUlJQkh3WUlLd1lCQlFVSEFRRUVnZ0VSTUlJQkRUQnZCZ2dyQmdFRkJRY3dBb1pqYUhSMGNEb3ZMMk5sY25ScFptbGpZWFJsY3pFdVltVnVkR3hsZVM1amIyMHZRMlZ5ZEVWdWNtOXNiQzlqWlhKMGFXWnBZMkYwWlhNeExtSmxiblJzWlhrdVkyOXRYMk5sY25ScFptbGpZWFJsY3pFdVltVnVkR3hsZVM1amIyMHVZM0owTUdjR0NDc0dBUVVGQnpBQ2hsdG9kSFJ3T2k4dlkyVnlkSEpsYm1WM1lXd3hMbUpsYm5Sc1pYa3VZMjl0TDJOeWJDOWpaWEowYVdacFkyRjBaWE14TG1KbGJuUnNaWGt1WTI5dFgyTmxjblJwWm1sallYUmxjekV1WW1WdWRHeGxlUzVqYjIwdVkzSjBNREVHQ0NzR0FRVUZCekFCaGlWb2RIUndPaTh2WTJWeWRHbG1hV05oZEdWek1TNWlaVzUwYkdWNUxtTnZiUzl2WTNOd01DRUdDU3NHQVFRQmdqY1VBZ1FVSGhJQVZ3QmxBR0lBVXdCbEFISUFkZ0JsQUhJd0RRWUpLb1pJaHZjTkFRRUZCUUFEZ2dFQkFMQUVzUk5MZms5cnpGeWVUNG5TWVVyY21oV0Z4dXRlc09hZnkxb3FXckxscjg2Ym9MVnZzdWN0YjdURW1SbFgwL09ySXdUZGRyellOSTNNdFBuRjJSSlg5clNtWE1ncEcvaitxcHA3UkF4c01pSGE1Uy8xZlBmR2RaUmRvT1BURHFmTTZXQVNjbXk1VlNCcVZHOGJzWW1ydk9QN0lhMnpSeEdPanJCNlBiTWhTbWVPbVA5MXgzV3FoZDlxTGxOL0VIR01BM0lSc3RhU1gzb3JkMGJqRmJOTDNBQVZsc25GSmpQSVluWTJCUFUvTmZjYTFXVVFqUDF3LzVwOEFldkV6Ty8rdldRd3JZOXE2KyszTjN6Mkw1eTE5eGlCdkJ5SFgzTlJBZ1NFMysrQTJyZE5zSmNXNlp5RWQ5RW1BejQveklrMWR0NlhpYmN4bXNYYm93RWNEKzg9PC9YNTA5Q2VydGlmaWNhdGU+PC9YNTA5RGF0YT48L0tleUluZm8+PC9kczpTaWduYXR1cmU+PC9zYW1sOkFzc2VydGlvbj4=";
    token.append(charToken);

    delete lpwstrToken;
    delete charToken;
    CCApi_FreeApi(api);

    std::queue<User*> inactiveUsers;
    std::default_random_engine  generator;
    std::uniform_real_distribution<double> distribution = std::uniform_real_distribution<double>(0.0, 1.0);

    if(!tester.trickle)
        {
        for(int i = 0; i < tester.userManager.m_userCount; i++)
            {
            tester.userManager.users.push_back(new GCSUser(&token, &generator, &distribution, i, &stats, tester.m_serverType));
            }
        }
    else
        {
        tester.userManager.users.push_back(new GCSUser(&token, &generator, &distribution, 0, &stats, tester.m_serverType)); //start with one
            for (int i = 1; i < tester.userManager.m_userCount; i++)
            {
            inactiveUsers.push(new GCSUser(&token, &generator, &distribution, i, &stats, tester.m_serverType)); //feed the rest to the Dispatcher
            }
        }

    tester.SetupInactiveUsers(inactiveUsers);

    tester.Main2(0);
    stats.WriteToFile(tester.userManager.m_userCount, tester.path);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                  12/2016
//+---------------+---------------+---------------+---------------+---------------+------*/
CURL* GCSUser::SetupCurl(Utf8StringCR url, Utf8StringCP retString, FILE* fp, Utf8StringCR postFields)
    {
    CURL *pCurl = NULL;

    m_correspondance.req.payload = postFields;

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
        headers = curl_slist_append(headers, (*(m_token)).c_str());

        curl_easy_setopt(pCurl, CURLOPT_URL, url);

        curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0);
        
        curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(pCurl, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);
        
        curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, WriteCallback);
        curl_easy_setopt(pCurl, CURLOPT_HEADERDATA, &(m_correspondance.response.header));

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

        m_start = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        }
    else
        m_correspondance.response.toolCode = CURLcode::CURLE_FAILED_INIT;

    return pCurl;
    }
