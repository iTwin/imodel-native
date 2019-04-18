/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <thread>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>
#include <queue>
#include <CCApi/CCPublic.h>

#include <Bentley/BeFile.h>
#include <RealityPlatformTools/RealityConversionTools.h>

#include "LoadTestBase.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

std::mutex inactiveUserMutex;
std::mutex statMutex;
std::mutex rpsMutex;

static std::queue<User*> s_inactiveUsers = std::queue<User*>();
static RPS* s_rps;
static bool s_keepRunning = true;
static bool s_startLogging = false;
static bool s_wrapUp = true;
static Stats* s_stats;

static int64_t s_statStartTime;
static int64_t s_ultimateStartTime;

static int s_targetRequestsPerHour;
static int s_totalRequests = 0;
static int s_sleepBiasMilliseconds = 0;

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String FullInfo::LogError() const
    {
    return Utf8PrintfString("Request %d :\n%s\n%s\n%s\nReponse:\n%lu\t%d\n%s\n+----------------------------------------------------------------+\n", id, req.url.c_str(), req.headers.c_str(), req.payload.c_str(), response.responseCode, response.toolCode, response.body.c_str());
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* request per second class
//* for each operation type, keep a cout of how many requests were sent at any given second
//+---------------+---------------+---------------+---------------+---------------+------*/
RPS::RPS() :requestLog(bmap<int, bmap<int64_t, int>>()){}

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void RPS::AddRequest(int type, int64_t time)
    {
    time /= 1000; //need seconds, not milliseconds
    std::lock_guard<std::mutex> lock(rpsMutex);
    requestLog[type][time] += 1;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
double RPS::GetRPS(int type, int64_t time)
    {
    int64_t amount = 0;
    time /= 1000; //need seconds, not milliseconds
    //get the average number of requests per second, over ten seconds
    for (int64_t i = (time - 12); i < (time - 2); i++)
        amount += requestLog[type][i];
    return amount / 10.0;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
size_t getInnactiveUserSize()
    {
    std::lock_guard<std::mutex> lock(inactiveUserMutex);
    return s_inactiveUsers.size();
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void restartUser(UserManager* manager)
    {
    std::lock_guard<std::mutex> lock(inactiveUserMutex);
    User* user = s_inactiveUsers.front();
    s_inactiveUsers.pop();

    if(user->DoNext(manager))
        s_inactiveUsers.push(user);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void Dispatch(UserManager* manager, int requestBuffer)
    {
    bool hatching = true;

    bool currentlyDecreasing = false;
    bool targetAttained = false;
    int increasingCount = 10;
    int decreasingCount = 10;
    int refreshTimer = 0;
    std::chrono::milliseconds emptyTimer(1000);

    while (s_keepRunning || s_wrapUp)
        {
        float inactiveUsers = (float)getInnactiveUserSize();
        if (inactiveUsers == 0)
            {
            hatching = false;
            std::this_thread::sleep_for(emptyTimer);
            continue;
            }

        restartUser(manager);

        if (s_targetRequestsPerHour > 0)
            {
            int64_t currentTime;
            DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);
            int requestsPerHour = (int)(3600.0 * (double)(s_totalRequests) / ((currentTime - s_ultimateStartTime) / 1000.0)) + 1; // +1 is to make sure value is not 0
            double deviationFactor = fabs(s_targetRequestsPerHour - requestsPerHour) / s_targetRequestsPerHour;
            int stepSeed = (s_sleepBiasMilliseconds < 50 ? 50 : s_sleepBiasMilliseconds);
            if (s_targetRequestsPerHour < requestsPerHour)
                {
                if (currentlyDecreasing)
                    {
                    // Trend just changed ... for the next 10 decrease we will be more agressive
                    increasingCount = 0;
                    currentlyDecreasing = false;
                    }

                increasingCount++;
                if (increasingCount <= 10)
                    s_sleepBiasMilliseconds += (int)(stepSeed * 4 * deviationFactor);  // Too fast ... increase sleep bias
                else
                    s_sleepBiasMilliseconds += (int)(stepSeed * deviationFactor);  // Too fast ... increase sleep bias
                }
            else if (s_sleepBiasMilliseconds > (stepSeed * deviationFactor))
                {
                if (!currentlyDecreasing)
                    {
                    // Trend just changed ... for the next 10 decrease we will be more agressive
                    decreasingCount = 0;
                    currentlyDecreasing = true;
                    }

                decreasingCount++;
                if (decreasingCount <= 10 && s_sleepBiasMilliseconds > (stepSeed * 4 * deviationFactor))
                    s_sleepBiasMilliseconds -= (int)(stepSeed * 4 * deviationFactor);  // Too slow ... decrease sleep bias
                else
                    s_sleepBiasMilliseconds -= (int)(stepSeed * deviationFactor);  // Too slow ... decrease sleep bias
                }

            refreshTimer += 100;
            std::chrono::milliseconds ms(s_sleepBiasMilliseconds);
            std::this_thread::sleep_for(ms);

            if (deviationFactor < 0.1)
                targetAttained = true;
            }
        else
            {
            int sleep = rand();
            float userCount = (float)(manager->m_userCount);
            if (!hatching)
                {
                if ((userCount - inactiveUsers) > 0.0000001)
                    sleep %= (int)(2100 * (1.0f - (inactiveUsers / userCount)));
                else
                    sleep = 0;
                }
            else
                sleep %= 600;

            refreshTimer += sleep;
            std::chrono::milliseconds basicTimer(sleep);
            std::this_thread::sleep_for(basicTimer);
            }

        if (!s_startLogging && (s_totalRequests > requestBuffer) && (s_targetRequestsPerHour == 0 || targetAttained))
            {
            s_startLogging = true;
            DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(s_statStartTime);
            }

        if (s_keepRunning && refreshTimer > 1000)
            {
            s_stats->PrintStats();
            refreshTimer = 0;
            }
        }
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void Terminate()
    {
    getch();
    s_keepRunning = false;
    std::cout << "exit requested, wrapping up pending requests and exiting..." << std::endl;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void Stat::Update(bool isSuccess, int64_t time)
    {
    int total = success + failure;
    if (isSuccess)
        success += 1;
    else
        failure += 1;

    minTime = std::min(minTime, time);
    maxTime = std::max(maxTime, time);
    avgTime = ((avgTime*total) + time) / (total + 1);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* for each operation type, tracks success and failures, as well as response time
//+---------------+---------------+---------------+---------------+---------------+------*/
Stats::Stats(RPS* rps): m_rps(rps) {}

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void Stats::InsertStats(const User* user, bool success, int activeUsers)
    {
    std::lock_guard<std::mutex> lock(statMutex);
    m_activeUsers = activeUsers;

    s_totalRequests += 1;
    if (s_startLogging)
        {
        opStats[user->m_currentOperation]->Update(success, user->m_lastRequestTimeMilliseconds);
        if (!success)
            errors[user->m_currentOperation].push_back(user->m_correspondance.LogError());
        }
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
size_t Stats::LogRequest(Utf8String req)
    {
    std::lock_guard<std::mutex> lock(statMutex);
    size_t size = opLog.size();
    opLog.push_back(Utf8PrintfString("%d\t%s", size, req.c_str()));
    return size;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void Stats::PrintStats()
    {
    int64_t currentTime;
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);
    std::lock_guard<std::mutex> statlock(statMutex);
    std::lock_guard<std::mutex> rpslock(rpsMutex);
    int totalSuccess, totalFailure;
    PrintStatsBody(currentTime, totalSuccess, totalFailure);

    std::cout << Utf8PrintfString("Total Time (seconds): %6d", (currentTime - s_statStartTime) / 1000) << std::endl;
    std::cout << Utf8PrintfString("Stat Total request/s:      %lf", (double)(totalSuccess + totalFailure) / ((currentTime - s_statStartTime) / 1000.0)) << std::endl;
    if (s_startLogging)
        {
        int requestsPerHour = (int)(3600.0 * (double)(s_totalRequests) / ((currentTime - s_ultimateStartTime) / 1000.0)) + 1; // +1 is to make sure value is not 0
        std::cout << Utf8PrintfString("Current request/hour:   %6d       TARGET:   %6d", requestsPerHour, s_targetRequestsPerHour) << std::endl;
        std::cout << "WARMING UP: STATS PENDING" << std::endl;
        }
    else
        std::cout << Utf8PrintfString("Total request/hour:   %lf", 3600.0 * (double)(s_totalRequests) / ((currentTime - s_ultimateStartTime) / 1000.0)) << std::endl;

    std::cout << Utf8PrintfString("Sleep bias (ms): %6d", s_sleepBiasMilliseconds) << std::endl;

    std::cout << "active users: " << m_activeUsers << std::endl;
    
    std::wcout << "inactive users: " << getInnactiveUserSize() << " (not synched, total may be incorrect)"<< std::endl << std::endl;

    std::cout << "Press any key to quit testing" << std::endl;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void Stats::WriteToFile(int userCount, Utf8String path)
    {
    time_t generatedFileName = std::time(nullptr);
    char name[64]; //the name given to the logfile is the system time upon exiting
    sprintf(name, "%d.log", (int)generatedFileName);
    path.append(name);
    std::ofstream file (path.c_str());
    file << userCount << " users" << std::endl;
    file << "target: " << s_targetRequestsPerHour << " requests per hour" << std::endl;
    file << s_totalRequests << " requests logged" << std::endl;
    int64_t currentTime;
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);
    file << "Runtime: " << ((currentTime - s_ultimateStartTime) / 1000.0) << " seconds" << std::endl;
    file << asctime(localtime(&generatedFileName)) << std::endl;
    file << "Type                 Success    Failure   minTime    maxTime   avgTime" << std::endl;
    WriteToFileBody(userCount, path, file);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void ShowUsage(int argc, char* argv[])
    {
    std::cout << "Server Load Tester" << std::endl << std::endl;
    std::cout << "Usage:" << argv[0] << "-s:[serverType] -u:[users]" << std::endl << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help                                      Show this help message and exit" << std::endl;
    std::cout << "  -s, --serverType        {dev, qa, prod}" << std::endl;
    std::cout << "  -u:[number of users], --users:[number of users] Number of users" << std::endl;
    std::cout << "  -hr:[rateperhour], --hourlyrate:[rateperhour]   Rate of requests per hour ex: 1000" << std::endl;
    std::cout << "  -t, --trickle                                   optional, add this argument to avoid spawing all users at once" << std::endl;
    std::cout << "  -p, --path                                      optional, specifiy a path to out the log file to" << std::endl;
    std::cout << "                                                  any text added after the last backslash will be added to the file name" << std::endl;
    std::cout << "  if there are spaces in an argument, surround it with \"\" " << std::endl;
    std::cout << "  as in \"-p:C:\\Program Files(...)\" " << std::endl;

    std::cout << std::endl << "Press any key to exit." << std::endl;
    getch();
    }
///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
User::User() :
    m_currentOperation(0), m_linked(false), m_wrappedUp(false)
    {}

User::User(int id, Stats* stats) :
    m_currentOperation(0), m_userId(id), m_linked(false), m_wrappedUp(false),
    m_fileName(BeFileName(Utf8PrintfString("%d", m_userId))), m_stats(stats)
    {}

bool User::DoNext(UserManager* owner)
    {
    if(!s_keepRunning)
        return WrapUp(owner);
    return DoNextBody(owner);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
UserManager::UserManager()
    {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    m_pCurlHandle = curl_multi_init();
    m_certPath = WString();
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
UserManager::~UserManager()
    {
    if (m_pCurlHandle != NULL)
        curl_multi_cleanup(m_pCurlHandle);
    curl_global_cleanup();
    for (int i = 0; i < users.size(); i++)
        delete users[i];
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void UserManager::Perform()
    {
    // we can optionally limit the total amount of connections this multi handle uses 
    curl_multi_setopt(m_pCurlHandle, CURLMOPT_MAXCONNECTS, m_userCount);

    for (int i = 0; i < std::min(m_userCount, (int)users.size()); ++i)
        {
        if(users[i]->DoNext(this))
            {
            std::lock_guard<std::mutex> lock(inactiveUserMutex);

            s_inactiveUsers.push(users[i]);
            }
        }

    int still_running; /* keep number of running handles */
    int repeats = 0;

    std::cout << "launching requests, it may take a few seconds to receive responses and display results" << std::endl;

    curl_multi_perform(m_pCurlHandle, &still_running);
    std::chrono::milliseconds waitTimer(100);
    do
        {
        CURLMcode mc; /* curl_multi_wait() return code */
        int numfds;

        /* wait for activity, timeout or "nothing" */
        mc = curl_multi_wait(m_pCurlHandle, NULL, 0, 1000, &numfds);

        if (mc != CURLM_OK)
            {
            std::cout << "curl_multi_wait() failed" << std::endl;
            break;
            }

        if (!numfds)
            {
            repeats++; /* count number of repeated zero numfds */
            if(repeats > 300) //something went wrong, after 30 seconds, exit
                {
                s_keepRunning = false;
                break;
                }
            else if (repeats > 1)
                std::this_thread::sleep_for(waitTimer); /* sleep 100 milliseconds */
            }
        else
            repeats = 0;

        CURLMsg *msg;
        int nbQueue;
        while ((msg = curl_multi_info_read(m_pCurlHandle, &nbQueue)))
            {
            char *pClient;
            curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &pClient);
            struct User *user = (struct User *)pClient;

            user->m_correspondance.response.toolCode = msg->data.result;
            curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &(user->m_correspondance.response.responseCode));


            if (msg->msg == CURLMSG_DONE)//response received, ensure that it is valid
                {
                double totalTime;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_TOTAL_TIME, &totalTime);
                user->m_lastRequestTimeMilliseconds = (int64_t)(totalTime * 1000.0);
                user->ValidatePrevious(still_running);
                }

            std::lock_guard<std::mutex> lock(inactiveUserMutex);

            s_inactiveUsers.push(user);

            curl_multi_remove_handle(m_pCurlHandle, msg->easy_handle);
            curl_easy_cleanup(msg->easy_handle);
            }

        if (s_wrapUp && !s_keepRunning)
            {
            Repopulate();
            }

        curl_multi_perform(m_pCurlHandle, &still_running);
        } while (s_wrapUp || still_running > 0);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* when the dispatcher isn't working quickly enough, restarts all inactive users
//+---------------+---------------+---------------+---------------+---------------+------*/
void UserManager::Repopulate()
    {
    std::lock_guard<std::mutex> lock(inactiveUserMutex);

    User* user;

    while (s_inactiveUsers.size() > 0)
        {
        user = s_inactiveUsers.front();
        s_inactiveUsers.pop();
        
        if (user->DoNext(this))
            s_inactiveUsers.push(user);
        }
    
    s_wrapUp = false;
    for (User* rdsUser : users)
        {
        if (!rdsUser->m_wrappedUp)
            {
            s_wrapUp = true;
            break;
            }
        }
    }

void UserManager::SetupCurl(CURL* curl, User* user)
    {
    curl_easy_setopt(curl, CURLOPT_PRIVATE, user);
    curl_multi_add_handle((CURLM*)m_pCurlHandle, curl);
    s_rps->AddRequest(user->m_currentOperation, user->m_start);
    }

LoadTester::LoadTester(): trickle(false), userManager(UserManager()), path(""){}

bool LoadTester::Main(int argc, char* argv[])
    {
    // Deactivate target requests per hour (run to max)
    s_targetRequestsPerHour = 0;

    if (argc < 2)
        {
        ShowUsage(argc, argv);
        return true;
        }

    char* substringPosition;
    int userCount = 0;

    // Default server type
    m_serverType = RealityPlatform::CONNECTServerType::QA;


    //parse command line arguments
    for (int i = 0; i < argc; ++i)
        {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            {
            ShowUsage(argc, argv);
            return true;
            }
        else if (strstr(argv[i], "-s:") || strstr(argv[i], "--serverType:"))
            {
            if(strstr(argv[i], "qa"))
                m_serverType = RealityPlatform::CONNECTServerType::QA;
            else if (strstr(argv[i], "prod"))
                m_serverType = RealityPlatform::CONNECTServerType::PROD;
            else if (strstr(argv[i], "perf"))
                m_serverType = RealityPlatform::CONNECTServerType::PERF;
            else
                m_serverType = RealityPlatform::CONNECTServerType::DEV;
            }
        else if (strstr(argv[i], "-u") || strstr(argv[i], "--users"))
            {
            substringPosition = strstr(argv[i], ":");

            if (substringPosition == 0)
                {
                ShowUsage(argc, argv);
                return true;
                }
            substringPosition++;
            Utf8String userString = Utf8String(substringPosition);
            uint64_t users;
            BeStringUtilities::ParseUInt64(users, userString.c_str());
            userCount = (int)users;
            }
        else if (strstr(argv[i], "-hr") || strstr(argv[i], "--hourlyrate"))
            {
            substringPosition = strstr(argv[i], ":");

            if (substringPosition == 0)
                {
                ShowUsage(argc, argv);
                return true;
                }
            substringPosition++;
            Utf8String userString = Utf8String(substringPosition);
            uint64_t hourlyRate;
            BeStringUtilities::ParseUInt64(hourlyRate, userString.c_str());
            s_targetRequestsPerHour = (int)hourlyRate;
            }
        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--trickle") == 0)
            {
            trickle = true;
            }
        else if (strstr(argv[i], "-p:") || strstr(argv[i], "--path:"))
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            path = Utf8String(substringPosition);
            }
        }

    if (s_targetRequestsPerHour == 0)
        s_startLogging = true;

    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(s_ultimateStartTime);
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(s_statStartTime);

    userManager.m_userCount = userCount;

    return false;
    }

void LoadTester::Main2(int requestBuffer)
    {
    std::thread dispatch(Dispatch, &userManager, requestBuffer);
    std::thread terminate(Terminate);

    userManager.Perform();
    std::cout << "---EXIT---" << std::endl;
    s_keepRunning = false;
    terminate.join();
    dispatch.join();
    }

void LoadTester::SetupStaticClasses(Stats* stats, RPS* rps)
    {
    s_stats = stats;
    s_rps = rps;
    }

void LoadTester::SetupInactiveUsers(std::queue<User*>& inactiveUsers) { s_inactiveUsers = inactiveUsers; }

Utf8String LoadTester::MakeBuddiCall(WString service, int region)
    {
    CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
    CallStatus status = APIERR_SUCCESS;

    bool installed;
    status = CCApi_IsInstalled(api, &installed);
    if (!installed)
        {
        std::cout << "Connection client does not seem to be installed\n" << std::endl;
        CCApi_FreeApi(api);
        return "";
        }
    bool running = false;
    status = CCApi_IsRunning(api, &running);
    if (status != APIERR_SUCCESS || !running)
        {
        std::cout << "Connection client does not seem to be running\n" << std::endl;
        CCApi_FreeApi(api);
        return "";
        }
    bool loggedIn = false;
    status = CCApi_IsLoggedIn(api, &loggedIn);
    if (status != APIERR_SUCCESS || !loggedIn)
        {
        std::cout << "Connection client does not seem to be logged in\n" << std::endl;
        CCApi_FreeApi(api);
        return "";
        }
    bool acceptedEula = false;
    status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
    if (status != APIERR_SUCCESS || !acceptedEula)
        {
        std::cout << "Connection client user does not seem to have accepted EULA\n" << std::endl;
        CCApi_FreeApi(api);
        return "";
        }
    bool sessionActive = false;
    status = CCApi_IsUserSessionActive(api, &sessionActive);
    if (status != APIERR_SUCCESS || !sessionActive)
        {
        std::cout << "Connection client does not seem to have an active session\n" << std::endl;
        CCApi_FreeApi(api);
        return "";
        }

    wchar_t* buddiUrl;
    UINT32 strlen = 0;

    if (region > 100)
        {
        CCApi_GetBuddiRegionUrl(api, service.c_str(), region, NULL, &strlen);
        strlen += 1;
        buddiUrl = (wchar_t*)malloc((strlen) * sizeof(wchar_t));
        CCApi_GetBuddiRegionUrl(api, service.c_str(), region, buddiUrl, &strlen);
        }
    else
        {
        CCApi_GetBuddiUrl(api, service.c_str(), NULL, &strlen);
        strlen += 1;
        buddiUrl = (wchar_t*)malloc((strlen) * sizeof(wchar_t));
        CCApi_GetBuddiUrl(api, service.c_str(), buddiUrl, &strlen);
        }

    char* charServer = new char[strlen];
    wcstombs(charServer, buddiUrl, strlen);

    CCApi_FreeApi(api);

    return Utf8String(charServer);
    }