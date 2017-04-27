#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <thread>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>

#include <Bentley/BeFile.h>
#include <RealityPlatform/RealityConversionTools.h>

#include "RDSLoadTest.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

std::mutex innactiveUserMutex;
std::mutex statMutex;
std::mutex rpsMutex;

static bvector<User*> s_innactiveUsers = bvector<User*>();
static RPS s_rps = RPS();
static bool s_keepRunning = true;
static Stats s_stats = Stats();

static int64_t s_startTime;

static int s_targetRequestsPerHour;

static int s_sleepBiasMilliseconds = 0;

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String FullInfo::LogError() const
    {
    return Utf8PrintfString("Request %d :\n%s\n%s\n%s\nReponse:\n%lu\t%d\n%s", id, req.url, req.headers, req.payload, response.responseCode, response.curlCode, response.body);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* request per second class
//* for each operation type, keep a cout of how many requests were sent at any given second
//+---------------+---------------+---------------+---------------+---------------+------*/
RPS::RPS():requestLog(bmap<OperationType, bmap<int64_t, int>>())
    {
    requestLog.Insert(OperationType::LIST_REALITYDATA, bmap<int64_t, int>());
    requestLog.Insert(OperationType::NAVNODE, bmap<int64_t, int>());
    requestLog.Insert(OperationType::ENTERPRISE_STAT, bmap<int64_t, int>());
    requestLog.Insert(OperationType::DETAILS, bmap<int64_t, int>());
    requestLog.Insert(OperationType::AZURE_ADDRESS, bmap<int64_t, int>());
    requestLog.Insert(OperationType::CREATE_REALITYDATA, bmap<int64_t, int>());
    requestLog.Insert(OperationType::MODIFY_REALITYDATA, bmap<int64_t, int>());
    requestLog.Insert(OperationType::CREATE_RELATIONSHIP, bmap<int64_t, int>());
    requestLog.Insert(OperationType::DELETE_RELATIONSHIP, bmap<int64_t, int>());
    requestLog.Insert(OperationType::DELETE_REALITYDATA, bmap<int64_t, int>());
    requestLog.Insert(OperationType::DOWNLOAD, bmap<int64_t, int>());
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void RPS::AddRequest(OperationType type, int64_t time)
    {
    std::lock_guard<std::mutex> lock(rpsMutex);
    requestLog[type][time] += 1;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
double RPS::GetRPS(OperationType type, int64_t time)
    {
    bmap<int64_t, int> times = requestLog[type];

    int64_t amount = 0;
    //get the average number of requests per second, over ten seconds
    for(int64_t i = (time - 12); i < (time - 2); i++)
        amount += times[i];
    return amount/10.0;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
size_t getInnactiveUserSize()
    {
    std::lock_guard<std::mutex> lock(innactiveUserMutex);
    return s_innactiveUsers.size();
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void restartUser(UserManager* manager)
    {
    std::lock_guard<std::mutex> lock(innactiveUserMutex);
    User* user = s_innactiveUsers.back();
    s_innactiveUsers.pop_back();
    user->DoNext(manager);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void Dispatch(UserManager* manager)
    {
    bool hatching = true;
    float userCount = (float)(manager->m_userCount);
    while (s_keepRunning)
        {
        float innactiveUsers = (float)getInnactiveUserSize();
        if (innactiveUsers == 0)
            {
            hatching = false;
            Sleep(2000);
            continue;
            }

        restartUser(manager);

        int sleep = rand();
        if(!hatching)
            {
            if((userCount-innactiveUsers) > 0.0000001)
                sleep %= (int)(2100 * (1.0f - (innactiveUsers/userCount)));
            else
                sleep = 0;
            if(s_keepRunning)
                s_stats.PrintStats();
            }
        else 
            sleep %= 600;

#if (0)
        // If a target requests per hour is set ...
        if (s_targetRequestsPerHour > 0)
            {
            int64_t currentTime;
            DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);
            int totalRequests = s_stats.GetTotalRequests();
            int requestsPerHour = (int)(3600.0 * (double)(totalRequests) / ((currentTime - s_startTime)/1000.0));
            if (s_targetRequestsPerHour < requestsPerHour)
                s_sleepBiasMilliseconds += 100;  // Too fast ... increase sleep bias
            else if (s_sleepBiasMilliseconds > 100)
                s_sleepBiasMilliseconds -= 100;  // Too slow ... decrease sleep bias

            sleep += s_sleepBiasMilliseconds;
            }
#endif
            
        Sleep(sleep);
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
    if(isSuccess)
        success+=1;
    else
        failure+=1;

    minTime = std::min(minTime, time);
    maxTime = std::max(maxTime, time);
    avgTime = ((avgTime*total) + time) / (total+1);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* for each operation type, tracks success and failures, as well as response time
//+---------------+---------------+---------------+---------------+---------------+------*/
Stats::Stats()
    {   
    opStats = bmap<OperationType, Stat*>();
    opStats.Insert(OperationType::LIST_REALITYDATA, new Stat());
    opStats.Insert(OperationType::NAVNODE, new Stat());
    opStats.Insert(OperationType::ENTERPRISE_STAT, new Stat());
    opStats.Insert(OperationType::DETAILS, new Stat());
    opStats.Insert(OperationType::AZURE_ADDRESS, new Stat());
    opStats.Insert(OperationType::CREATE_REALITYDATA, new Stat());
    opStats.Insert(OperationType::MODIFY_REALITYDATA, new Stat());
    opStats.Insert(OperationType::CREATE_RELATIONSHIP, new Stat());
    opStats.Insert(OperationType::DELETE_RELATIONSHIP, new Stat());
    opStats.Insert(OperationType::DELETE_REALITYDATA, new Stat());
    opStats.Insert(OperationType::DOWNLOAD, new Stat());

    errors = bmap<OperationType, bvector<Utf8String>>();
    errors.Insert(OperationType::LIST_REALITYDATA, bvector<Utf8String>());
    errors.Insert(OperationType::NAVNODE, bvector<Utf8String>());
    errors.Insert(OperationType::ENTERPRISE_STAT, bvector<Utf8String>());
    errors.Insert(OperationType::DETAILS, bvector<Utf8String>());
    errors.Insert(OperationType::AZURE_ADDRESS, bvector<Utf8String>());
    errors.Insert(OperationType::CREATE_REALITYDATA, bvector<Utf8String>());
    errors.Insert(OperationType::MODIFY_REALITYDATA, bvector<Utf8String>());
    errors.Insert(OperationType::CREATE_RELATIONSHIP, bvector<Utf8String>());
    errors.Insert(OperationType::DELETE_RELATIONSHIP, bvector<Utf8String>());
    errors.Insert(OperationType::DELETE_REALITYDATA, bvector<Utf8String>());
    errors.Insert(OperationType::DOWNLOAD, bvector<Utf8String>());
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void Stats::InsertStats(const User* user, bool success, int activeUsers)
    {
    std::lock_guard<std::mutex> lock(statMutex);
    m_activeUsers = activeUsers;
    int64_t endTime;
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(endTime);
    opStats[user->m_currentOperation]->Update(success, endTime - user->m_start);
    if(!success)
        errors[user->m_currentOperation].push_back(user->m_correspondance.LogError());

#if (0)
    // We use the insert stat as delay
    // If a target requests per hour is set ...
    if (s_targetRequestsPerHour > 0)
        {
        int64_t currentTime;
        DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);
        int totalRequests = s_stats.GetTotalRequests();
        int requestsPerHour = (int)(3600.0 * (double)(totalRequests) / ((currentTime - s_startTime)/1000.0)) + 1; // +1 is to make sure value is not 0
        double deviationFactor = fabs((s_targetRequestsPerHour - requestsPerHour) / s_targetRequestsPerHour);
        if (s_targetRequestsPerHour < requestsPerHour)
            s_sleepBiasMilliseconds += (int)(100 * deviationFactor);  // Too fast ... increase sleep bias
        else if (s_sleepBiasMilliseconds > 100)
            s_sleepBiasMilliseconds -= (int)(100 * deviationFactor);  // Too slow ... decrease sleep bias

        Sleep(s_sleepBiasMilliseconds);
        }
#endif

    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
size_t Stats::LogRequest(Utf8String req) 
    { 
    std::lock_guard<std::mutex> lock(statMutex); 
    size_t size = opLog.size();
    opLog.push_back(Utf8PrintfString("%d\t%s", size, req)); 
    return size; 
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void Stats::PrintStats()
    {
    std::lock_guard<std::mutex> statlock(statMutex);
    std::lock_guard<std::mutex> rpslock(rpsMutex);
    int64_t currentTime;
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);
    system("cls");

    std::cout << "Type                    Success    Failure   minTime   maxTime   avgTime  requests/second" << std::endl;
    
    std::cout << Utf8PrintfString("Create Reality Data   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CREATE_REALITYDATA]->success, opStats[OperationType::CREATE_REALITYDATA]->failure, (int)opStats[OperationType::CREATE_REALITYDATA]->minTime, (int)opStats[OperationType::CREATE_REALITYDATA]->maxTime, (int)opStats[OperationType::CREATE_REALITYDATA]->avgTime, s_rps.GetRPS(OperationType::CREATE_REALITYDATA, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Create Relationship   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CREATE_RELATIONSHIP]->success, opStats[OperationType::CREATE_RELATIONSHIP]->failure, (int)opStats[OperationType::CREATE_RELATIONSHIP]->minTime, (int)opStats[OperationType::CREATE_RELATIONSHIP]->maxTime, (int)opStats[OperationType::CREATE_RELATIONSHIP]->avgTime, s_rps.GetRPS(OperationType::CREATE_RELATIONSHIP, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Details               %6d %10d %9d %10d %9d        %f", opStats[OperationType::DETAILS]->success, opStats[OperationType::DETAILS]->failure, (int)opStats[OperationType::DETAILS]->minTime, (int)opStats[OperationType::DETAILS]->maxTime, (int)opStats[OperationType::DETAILS]->avgTime, s_rps.GetRPS(OperationType::DETAILS, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("List Reality Data     %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_REALITYDATA]->success, opStats[OperationType::LIST_REALITYDATA]->failure, (int)opStats[OperationType::LIST_REALITYDATA]->minTime, (int)opStats[OperationType::LIST_REALITYDATA]->maxTime, (int)opStats[OperationType::LIST_REALITYDATA]->avgTime, s_rps.GetRPS(OperationType::LIST_REALITYDATA, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Enterprise Stat       %6d %10d %9d %10d %9d        %f", opStats[OperationType::ENTERPRISE_STAT]->success, opStats[OperationType::ENTERPRISE_STAT]->failure, (int)opStats[OperationType::ENTERPRISE_STAT]->minTime, (int)opStats[OperationType::ENTERPRISE_STAT]->maxTime, (int)opStats[OperationType::ENTERPRISE_STAT]->avgTime, s_rps.GetRPS(OperationType::ENTERPRISE_STAT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Change Reality Data   %6d %10d %9d %10d %9d        %f", opStats[OperationType::MODIFY_REALITYDATA]->success, opStats[OperationType::MODIFY_REALITYDATA]->failure, (int)opStats[OperationType::MODIFY_REALITYDATA]->minTime, (int)opStats[OperationType::MODIFY_REALITYDATA]->maxTime, (int)opStats[OperationType::MODIFY_REALITYDATA]->avgTime, s_rps.GetRPS(OperationType::MODIFY_REALITYDATA, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Delete Relationship   %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_RELATIONSHIP]->success, opStats[OperationType::DELETE_RELATIONSHIP]->failure, (int)opStats[OperationType::DELETE_RELATIONSHIP]->minTime, (int)opStats[OperationType::DELETE_RELATIONSHIP]->maxTime, (int)opStats[OperationType::DELETE_RELATIONSHIP]->avgTime, s_rps.GetRPS(OperationType::DELETE_RELATIONSHIP, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Delete Reality Data   %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_REALITYDATA]->success, opStats[OperationType::DELETE_REALITYDATA]->failure, (int)opStats[OperationType::DELETE_REALITYDATA]->minTime, (int)opStats[OperationType::DELETE_REALITYDATA]->maxTime, (int)opStats[OperationType::DELETE_REALITYDATA]->avgTime, s_rps.GetRPS(OperationType::DELETE_REALITYDATA, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("NavNode               %6d %10d %9d %10d %9d        %f", opStats[OperationType::NAVNODE]->success, opStats[OperationType::NAVNODE]->failure, (int)opStats[OperationType::NAVNODE]->minTime, (int)opStats[OperationType::NAVNODE]->maxTime, (int)opStats[OperationType::NAVNODE]->avgTime, s_rps.GetRPS(OperationType::NAVNODE, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("AzureAddress          %6d %10d %9d %10d %9d        %f", opStats[OperationType::AZURE_ADDRESS]->success, opStats[OperationType::AZURE_ADDRESS]->failure, (int)opStats[OperationType::AZURE_ADDRESS]->minTime, (int)opStats[OperationType::AZURE_ADDRESS]->maxTime, (int)opStats[OperationType::AZURE_ADDRESS]->avgTime, s_rps.GetRPS(OperationType::AZURE_ADDRESS, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Download              %6d %10d %9d %10d %9d        %f", opStats[OperationType::DOWNLOAD]->success, opStats[OperationType::DOWNLOAD]->failure, (int)opStats[OperationType::DOWNLOAD]->minTime, (int)opStats[OperationType::DOWNLOAD]->maxTime, (int)opStats[OperationType::DOWNLOAD]->avgTime, s_rps.GetRPS(OperationType::DOWNLOAD, currentTime)) << std::endl;
    std::cout << std::endl;
    int totalSuccess = opStats[OperationType::CREATE_REALITYDATA]->success + opStats[OperationType::CREATE_RELATIONSHIP]->success +
                       opStats[OperationType::DETAILS]->success + opStats[OperationType::LIST_REALITYDATA]->success +
                       opStats[OperationType::ENTERPRISE_STAT]->success + opStats[OperationType::MODIFY_REALITYDATA]->success +
                       opStats[OperationType::DELETE_RELATIONSHIP]->success + opStats[OperationType::DELETE_REALITYDATA]->success +
                       opStats[OperationType::NAVNODE]->success + opStats[OperationType::AZURE_ADDRESS]->success +
                       opStats[OperationType::DOWNLOAD]->success;

    int totalFailure = opStats[OperationType::CREATE_REALITYDATA]->failure + opStats[OperationType::CREATE_RELATIONSHIP]->failure +
                       opStats[OperationType::DETAILS]->failure + opStats[OperationType::LIST_REALITYDATA]->failure +
                       opStats[OperationType::ENTERPRISE_STAT]->failure + opStats[OperationType::MODIFY_REALITYDATA]->failure +
                       opStats[OperationType::DELETE_RELATIONSHIP]->failure + opStats[OperationType::DELETE_REALITYDATA]->failure +
                       opStats[OperationType::NAVNODE]->failure + opStats[OperationType::AZURE_ADDRESS]->failure +
                       opStats[OperationType::DOWNLOAD]->failure;      

    std::cout << Utf8PrintfString("Total                 %6d %10d", totalSuccess, totalFailure) << std::endl;
    std::cout << Utf8PrintfString("Total Requests        %6d", totalSuccess + totalFailure) << std::endl;

    std::cout << std::endl;

    std::cout << Utf8PrintfString("Total Time (seconds): %6d", (currentTime - s_startTime)/1000) << std::endl;
    std::cout << Utf8PrintfString("Total request/s:      %lf", (double)(totalSuccess + totalFailure) / ((currentTime - s_startTime)/1000.0)) << std::endl;
    std::cout << Utf8PrintfString("Total request/hour:   %lf", 3600.0 * (double)(totalSuccess + totalFailure) / ((currentTime - s_startTime)/1000.0)) << std::endl;
    std::cout << "active users: " << m_activeUsers << std::endl << std::endl;

    std::cout << "Press any key to quit testing" << std::endl;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
int Stats::GetTotalRequests()
    {
    int totalSuccess = opStats[OperationType::CREATE_REALITYDATA]->success + opStats[OperationType::CREATE_RELATIONSHIP]->success +
                       opStats[OperationType::DETAILS]->success + opStats[OperationType::LIST_REALITYDATA]->success +
                       opStats[OperationType::ENTERPRISE_STAT]->success + opStats[OperationType::MODIFY_REALITYDATA]->success +
                       opStats[OperationType::DELETE_RELATIONSHIP]->success + opStats[OperationType::DELETE_REALITYDATA]->success +
                       opStats[OperationType::NAVNODE]->success + opStats[OperationType::AZURE_ADDRESS]->success +
                       opStats[OperationType::DOWNLOAD]->success;

    int totalFailure = opStats[OperationType::CREATE_REALITYDATA]->failure + opStats[OperationType::CREATE_RELATIONSHIP]->failure +
                       opStats[OperationType::DETAILS]->failure + opStats[OperationType::LIST_REALITYDATA]->failure +
                       opStats[OperationType::ENTERPRISE_STAT]->failure + opStats[OperationType::MODIFY_REALITYDATA]->failure +
                       opStats[OperationType::DELETE_RELATIONSHIP]->failure + opStats[OperationType::DELETE_REALITYDATA]->failure +
                       opStats[OperationType::NAVNODE]->failure + opStats[OperationType::AZURE_ADDRESS]->failure +
                       opStats[OperationType::DOWNLOAD]->failure;      

    return (totalSuccess + totalFailure);
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
    file << asctime(localtime(&generatedFileName)) << std::endl;
    file << "Type        Success    Failure   minTime   maxTime   avgTime" << std::endl;
    
    int64_t currentTime;
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);

    file << Utf8PrintfString("Create Reality Data   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CREATE_REALITYDATA]->success, opStats[OperationType::CREATE_REALITYDATA]->failure, (int)opStats[OperationType::CREATE_REALITYDATA]->minTime, (int)opStats[OperationType::CREATE_REALITYDATA]->maxTime, (int)opStats[OperationType::CREATE_REALITYDATA]->avgTime, s_rps.GetRPS(OperationType::CREATE_REALITYDATA, currentTime)) << std::endl;
    file << Utf8PrintfString("Create Relationship   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CREATE_RELATIONSHIP]->success, opStats[OperationType::CREATE_RELATIONSHIP]->failure, (int)opStats[OperationType::CREATE_RELATIONSHIP]->minTime, (int)opStats[OperationType::CREATE_RELATIONSHIP]->maxTime, (int)opStats[OperationType::CREATE_RELATIONSHIP]->avgTime, s_rps.GetRPS(OperationType::CREATE_RELATIONSHIP, currentTime)) << std::endl;
    file << Utf8PrintfString("Details               %6d %10d %9d %10d %9d        %f", opStats[OperationType::DETAILS]->success, opStats[OperationType::DETAILS]->failure, (int)opStats[OperationType::DETAILS]->minTime, (int)opStats[OperationType::DETAILS]->maxTime, (int)opStats[OperationType::DETAILS]->avgTime, s_rps.GetRPS(OperationType::DETAILS, currentTime)) << std::endl;
    file << Utf8PrintfString("List Reality Data     %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_REALITYDATA]->success, opStats[OperationType::LIST_REALITYDATA]->failure, (int)opStats[OperationType::LIST_REALITYDATA]->minTime, (int)opStats[OperationType::LIST_REALITYDATA]->maxTime, (int)opStats[OperationType::LIST_REALITYDATA]->avgTime, s_rps.GetRPS(OperationType::LIST_REALITYDATA, currentTime)) << std::endl;
    file << Utf8PrintfString("Enterprise Stat       %6d %10d %9d %10d %9d        %f", opStats[OperationType::ENTERPRISE_STAT]->success, opStats[OperationType::ENTERPRISE_STAT]->failure, (int)opStats[OperationType::ENTERPRISE_STAT]->minTime, (int)opStats[OperationType::ENTERPRISE_STAT]->maxTime, (int)opStats[OperationType::ENTERPRISE_STAT]->avgTime, s_rps.GetRPS(OperationType::ENTERPRISE_STAT, currentTime)) << std::endl;
    file << Utf8PrintfString("Change Reality Data   %6d %10d %9d %10d %9d        %f", opStats[OperationType::MODIFY_REALITYDATA]->success, opStats[OperationType::MODIFY_REALITYDATA]->failure, (int)opStats[OperationType::MODIFY_REALITYDATA]->minTime, (int)opStats[OperationType::MODIFY_REALITYDATA]->maxTime, (int)opStats[OperationType::MODIFY_REALITYDATA]->avgTime, s_rps.GetRPS(OperationType::MODIFY_REALITYDATA, currentTime)) << std::endl;
    file << Utf8PrintfString("Delete Relationship   %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_RELATIONSHIP]->success, opStats[OperationType::DELETE_RELATIONSHIP]->failure, (int)opStats[OperationType::DELETE_RELATIONSHIP]->minTime, (int)opStats[OperationType::DELETE_RELATIONSHIP]->maxTime, (int)opStats[OperationType::DELETE_RELATIONSHIP]->avgTime, s_rps.GetRPS(OperationType::DELETE_RELATIONSHIP, currentTime)) << std::endl;
    file << Utf8PrintfString("Delete Reality Data   %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_REALITYDATA]->success, opStats[OperationType::DELETE_REALITYDATA]->failure, (int)opStats[OperationType::DELETE_REALITYDATA]->minTime, (int)opStats[OperationType::DELETE_REALITYDATA]->maxTime, (int)opStats[OperationType::DELETE_REALITYDATA]->avgTime, s_rps.GetRPS(OperationType::DELETE_REALITYDATA, currentTime)) << std::endl;
    file << Utf8PrintfString("NavNode               %6d %10d %9d %10d %9d        %f", opStats[OperationType::NAVNODE]->success, opStats[OperationType::NAVNODE]->failure, (int)opStats[OperationType::NAVNODE]->minTime, (int)opStats[OperationType::NAVNODE]->maxTime, (int)opStats[OperationType::NAVNODE]->avgTime, s_rps.GetRPS(OperationType::NAVNODE, currentTime)) << std::endl;
    file << Utf8PrintfString("AzureAddress          %6d %10d %9d %10d %9d        %f", opStats[OperationType::AZURE_ADDRESS]->success, opStats[OperationType::AZURE_ADDRESS]->failure, (int)opStats[OperationType::AZURE_ADDRESS]->minTime, (int)opStats[OperationType::AZURE_ADDRESS]->maxTime, (int)opStats[OperationType::AZURE_ADDRESS]->avgTime, s_rps.GetRPS(OperationType::AZURE_ADDRESS, currentTime)) << std::endl;
    file << Utf8PrintfString("Download              %6d %10d %9d %10d %9d        %f", opStats[OperationType::DOWNLOAD]->success, opStats[OperationType::DOWNLOAD]->failure, (int)opStats[OperationType::DOWNLOAD]->minTime, (int)opStats[OperationType::DOWNLOAD]->maxTime, (int)opStats[OperationType::DOWNLOAD]->avgTime, s_rps.GetRPS(OperationType::DOWNLOAD, currentTime)) << std::endl;

    file << std::endl << std::endl << "op list:" << std::endl;

    for (Utf8String op : opLog)
        file << op << std::endl;

    file << std::endl << std::endl << "error list:" << std::endl;
    
    file << "List Reality Data:" << std::endl;
    for (Utf8String error : errors[OperationType::LIST_REALITYDATA])
        file << error << std::endl;

    file << "NavNode:" << std::endl;
    for (Utf8String error : errors[OperationType::NAVNODE])
        file << error << std::endl;

    file << "Enterprise Stat:" << std::endl;
    for (Utf8String error : errors[OperationType::ENTERPRISE_STAT])
        file << error << std::endl;

    file << "Details:" << std::endl;
    for (Utf8String error : errors[OperationType::DETAILS])
        file << error << std::endl;

    file << "AzureAddress:" << std::endl;
    for (Utf8String error : errors[OperationType::AZURE_ADDRESS])
        file << error << std::endl;

    file << "Create Reality Data:" << std::endl;
    for (Utf8String error : errors[OperationType::CREATE_REALITYDATA])
        file << error << std::endl;

    file << "Create:" << std::endl;
    for (Utf8String error : errors[OperationType::CREATE_RELATIONSHIP])
        file << error << std::endl;

    file << "Delete Relationship:" << std::endl;
    for (Utf8String error : errors[OperationType::DELETE_RELATIONSHIP])
        file << error << std::endl;

    file << "Delete Reality Data:" << std::endl;
    for (Utf8String error : errors[OperationType::DELETE_REALITYDATA])
        file << error << std::endl;

    file << "Download:" << std::endl;
    for (Utf8String error : errors[OperationType::DOWNLOAD])
        file << error << std::endl;

    file.close();
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
User::User():
    m_currentOperation(OperationType::NAVNODE), m_handshake(AzureHandshake()), m_node(nullptr)
    {}

User::User(int id) :
    m_currentOperation(OperationType::NAVNODE), m_handshake(AzureHandshake()), m_node(nullptr),
    m_userId(id), m_fileName(BeFileName(Utf8PrintfString("%d", m_userId)))
{}

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* prepares next request, based on most recent action
//* can be called blindly from outside
//+---------------+---------------+---------------+---------------+---------------+------*/
void User::DoNext(UserManager* owner)
    {
    if(!s_keepRunning)
        return WrapUp(owner);

    m_currentOperation = static_cast<OperationType>(rand() % OperationType::last);

    CURL* curl = nullptr;
    if(m_currentOperation == OperationType::DETAILS)
        {
        if(m_node != nullptr)
            curl = Details();
        else
            m_currentOperation = OperationType::NAVNODE;
        }
    else if (m_currentOperation == OperationType::AZURE_ADDRESS)
            {
            if (m_node != nullptr)
                curl = AzureAddress();
            else
                m_currentOperation = OperationType::NAVNODE;
            }
    else if (m_currentOperation == OperationType::MODIFY_REALITYDATA)
            {
            if (m_id.empty())
                m_currentOperation = OperationType::CREATE_REALITYDATA;
            else
                curl = ModifyRealityData();
            }
    else if (m_currentOperation == OperationType::CREATE_RELATIONSHIP)
            {
            if (m_id.empty())
                m_currentOperation = OperationType::CREATE_REALITYDATA;
            else 
                {
                if (m_linked)
                    {
                    m_currentOperation = OperationType::DELETE_RELATIONSHIP;
                    curl = DeleteRelationship();
                    }
                else
                    curl = CreateRelationship();
                }
            }
    else if (m_currentOperation == OperationType::DELETE_RELATIONSHIP)
            {
            if (m_id.empty())
                m_currentOperation = OperationType::CREATE_REALITYDATA;
            else
                {
                if (!m_linked)
                    {
                    m_currentOperation = OperationType::CREATE_RELATIONSHIP;
                    curl = CreateRelationship();
                    }
                else
                    curl = DeleteRelationship();
                }
            }
    else if (m_currentOperation == OperationType::DELETE_REALITYDATA)
            {
            if (m_id.empty())
                m_currentOperation = OperationType::CREATE_REALITYDATA;
            else
                curl = DeleteRealityData();
            }
    else if (m_currentOperation == OperationType::LIST_REALITYDATA)
            {
            curl = ListRealityData();
            }
    else if (m_currentOperation == OperationType::ENTERPRISE_STAT)
            {
            curl = EnterpriseStat();
            }
    else if (m_currentOperation == OperationType::DOWNLOAD)
            {
            if (m_node != nullptr && m_node->GetClassName() == "Document")
                curl = Download();
            else
                m_currentOperation = OperationType::NAVNODE;
            }

    if (m_currentOperation == OperationType::NAVNODE)
            {
            curl = NavNodeFunc();
            }
    else if (m_currentOperation == OperationType::CREATE_REALITYDATA)
            {
            if(m_id.empty())
                curl = CreateRealityData();
            else
                {
                m_currentOperation = OperationType::DELETE_REALITYDATA;
                curl = DeleteRealityData();
                }
            }

#if (0)
    // We use the insert stat as delay
    // If a target requests per hour is set ...
    if (s_targetRequestsPerHour > 0)
        {
        int64_t currentTime;
        DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);
        int totalRequests = s_stats.GetTotalRequests();
        int requestsPerHour = (int)(3600.0 * (double)(totalRequests) / ((currentTime - s_startTime)/1000.0)) + 1; // +1 is to make sure value is not 0
        double deviationFactor = fabs((s_targetRequestsPerHour - requestsPerHour) / s_targetRequestsPerHour);
        if (s_targetRequestsPerHour < requestsPerHour)
            s_sleepBiasMilliseconds += (int)(100 * deviationFactor);  // Too fast ... increase sleep bias
        else if (s_sleepBiasMilliseconds > 100)
            s_sleepBiasMilliseconds -= (int)(100 * deviationFactor);  // Too slow ... decrease sleep bias

        Sleep(s_sleepBiasMilliseconds);
        }
#endif

    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(m_start);
    if(curl != nullptr)
        owner->SetupCurl(curl, this);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* prepares next request, based on most recent action
//* can be called blindly from outside
//+---------------+---------------+---------------+---------------+---------------+------*/
void User::WrapUp(UserManager* owner)
    {
    if (m_id.empty())
        return;

    m_currentOperation = OperationType::DELETE_REALITYDATA;
    CURL* curl = DeleteRealityData();
    
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(m_start);
    if (curl != nullptr)
        owner->SetupCurl(curl, this);
    }

CURL* User::ListRealityData()
    {
    RealityDataListByEnterprisePagedRequest enterpriseReq = RealityDataListByEnterprisePagedRequest("", 0, 25);

    enterpriseReq.SetFilter(RealityDataFilterCreator::FilterBySize(100, 100000));

    m_correspondance.response.clear();
    m_correspondance.req.url = enterpriseReq.GetHttpRequestString();
    m_correspondance.req.headers = enterpriseReq.GetRequestHeaders();
    m_correspondance.req.payload = enterpriseReq.GetRequestPayload();

    m_correspondance.id = s_stats.LogRequest("List Reality Data");

    return WSGRequest::GetInstance().PrepareRequest(enterpriseReq, m_correspondance.response, false, nullptr);
    }

void User::ValidateListRealityData(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    s_stats.InsertStats(this, 
        (m_correspondance.response.ValidateJSONResponse(instances, "instances") == RequestStatus::OK || m_correspondance.response.status == RequestStatus::LASTPAGE),
        activeUsers);
    }

CURL* User::NavNodeFunc()
    {
    m_correspondance.response.clear();
    if (m_node == nullptr || (m_node != nullptr && m_node->GetClassName() == "Document"))
        {
        delete m_node;
        m_node = nullptr;

        WSGNavRootRequest navRoot = WSGNavRootRequest(RealityDataService::GetServerName(), RealityDataService::GetWSGProtocol(), RealityDataService::GetRepoName());

        m_correspondance.req.url = navRoot.GetHttpRequestString();
        m_correspondance.req.headers = navRoot.GetRequestHeaders();
        m_correspondance.req.payload = navRoot.GetRequestPayload();

        m_correspondance.id = s_stats.LogRequest("navRoot");

        return WSGRequest::GetInstance().PrepareRequest(navRoot, m_correspondance.response, false, nullptr);
        }
    else
        {
        Utf8String nodePath = m_node->GetNavString();
        nodePath.ReplaceAll("/", "~2F");

        RawServerResponse versionResponse = RawServerResponse();
        WSGNavNodeRequest navNode = WSGNavNodeRequest(RealityDataService::GetServerName(), RealityDataService::GetWSGProtocol(), RealityDataService::GetRepoName(), nodePath);

        m_correspondance.req.url = navNode.GetHttpRequestString();
        m_correspondance.req.headers = navNode.GetRequestHeaders();
        m_correspondance.req.payload = navNode.GetRequestPayload();

        m_correspondance.id = s_stats.LogRequest(Utf8PrintfString("navNode for %s", nodePath));

        return WSGRequest::GetInstance().PrepareRequest(navNode, m_correspondance.response, false, nullptr);
        }
    }

void User::ValidateNavNode(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    bool success = (m_correspondance.response.ValidateJSONResponse(instances, "instances") == RequestStatus::OK);
    if(success == false && (m_correspondance.response.body.Contains("not listable") || m_correspondance.response.body.Contains("container does not exist")))
        success = true; //expected behaviour

    s_stats.InsertStats(this,
        success,
        activeUsers);

    if(m_correspondance.response.status == RequestStatus::OK && instances["instances"].size() > 0)
        {
        NavNode* newNode = new NavNode(instances["instances"][rand() % instances["instances"].size()], (m_node==nullptr) ? "" : m_node->GetRootNode(), (m_node == nullptr) ? "" : m_node->GetRootId());
        std::swap(m_node, newNode);
        delete newNode;
        } 
    else
        {
        delete m_node;
        m_node = nullptr;
        }
    }

CURL*  User::EnterpriseStat()
    {
    m_correspondance.response.clear();
    RealityDataEnterpriseStatRequest enterpriseReq = RealityDataEnterpriseStatRequest("");
    m_correspondance.req.url = enterpriseReq.GetHttpRequestString();
    m_correspondance.req.headers = enterpriseReq.GetRequestHeaders();
    m_correspondance.req.payload = enterpriseReq.GetRequestPayload();

    m_correspondance.id = s_stats.LogRequest("stat");

    return WSGRequest::GetInstance().PrepareRequest(enterpriseReq, m_correspondance.response, false, nullptr);
    }

void User::ValidateEnterpriseStat(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    s_stats.InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "instances") == RequestStatus::OK),
        activeUsers);
    }

CURL*  User::Details()
    {
    m_correspondance.response.clear();
    
    Utf8String className = m_node->GetClassName();

    Utf8String instanceId = m_node->GetInstanceId();
    instanceId.ReplaceAll("/", "~2F");
        
    if  (className == "Document")
        {
        RealityDataDocumentByIdRequest documentReq = RealityDataDocumentByIdRequest(instanceId);
        m_correspondance.req.url = documentReq.GetHttpRequestString();
        m_correspondance.req.headers = documentReq.GetRequestHeaders();
        m_correspondance.req.payload = documentReq.GetRequestPayload();

        m_correspondance.id = s_stats.LogRequest(Utf8PrintfString("document for %s", instanceId));

        return WSGRequest::GetInstance().PrepareRequest(documentReq, m_correspondance.response, false, nullptr);
        }   
    else if (className == "Folder")
        {
        RealityDataFolderByIdRequest folderReq = RealityDataFolderByIdRequest(instanceId);
        m_correspondance.req.url = folderReq.GetHttpRequestString();
        m_correspondance.req.headers = folderReq.GetRequestHeaders();
        m_correspondance.req.payload = folderReq.GetRequestPayload();

        m_correspondance.id = s_stats.LogRequest(Utf8PrintfString("folder for %s", instanceId));

        return WSGRequest::GetInstance().PrepareRequest(folderReq, m_correspondance.response, false, nullptr);
        }
    else if (className == "RealityData")
        {
        RealityDataByIdRequest idReq = RealityDataByIdRequest(instanceId);
        m_correspondance.req.url = idReq.GetHttpRequestString();
        m_correspondance.req.headers = idReq.GetRequestHeaders();
        m_correspondance.req.payload = idReq.GetRequestPayload();

        m_correspondance.id = s_stats.LogRequest(Utf8PrintfString("RealityData for %s", instanceId));

        return WSGRequest::GetInstance().PrepareRequest(idReq, m_correspondance.response, false, nullptr);
        }
    return nullptr;
    }

void User::ValidateDetails(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    s_stats.InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "instances") == RequestStatus::OK),
        activeUsers);
    }

CURL* User::AzureAddress()
    {
    m_handshake = AzureHandshake(m_node->GetInstanceId(), false);

    m_correspondance.response = RawServerResponse();
    m_correspondance.req.url = m_handshake.GetHttpRequestString();
    m_correspondance.req.headers = m_handshake.GetRequestHeaders();
    m_correspondance.req.payload = m_handshake.GetRequestPayload();

    m_correspondance.id = s_stats.LogRequest(Utf8PrintfString("azure address for %s -  user %d", m_node->GetInstanceId(), m_userId));

    return WSGRequest::GetInstance().PrepareRequest(m_handshake, m_correspondance.response, false, nullptr);
    }

void User::ValidateAzureAddress(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    s_stats.InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "instances") == RequestStatus::OK),
        activeUsers);
    }

CURL*  User::CreateRealityData()
    {
    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    
    properties.Insert(RealityDataField::Name, "Load Test (ERASE)");

    properties.Insert(RealityDataField::Classification, "MODEL");

    properties.Insert(RealityDataField::Type, "3DTiles");

    properties.Insert(RealityDataField::Visibility, "PUBLIC");

    RealityDataCreateRequest createRequest = RealityDataCreateRequest("", RealityDataServiceUpload::PackageProperties(properties));
    m_correspondance.response.clear();
    m_correspondance.req.url = createRequest.GetHttpRequestString();
    m_correspondance.req.headers = createRequest.GetRequestHeaders();
    m_correspondance.req.payload = createRequest.GetRequestPayload();

    m_correspondance.id = s_stats.LogRequest(Utf8PrintfString("Create Reality Data for user %d", m_userId));

    return WSGRequest::GetInstance().PrepareRequest(createRequest, m_correspondance.response, false, nullptr);
    }

void User::ValidateCreateRealityData(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    s_stats.InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "changedInstance") == RequestStatus::OK),
        activeUsers);

    if(m_correspondance.response.status == RequestStatus::OK)
        m_id = instances["changedInstance"]["instanceAfterChange"]["instanceId"].asString();

    m_linked = false;
    }

CURL*  User::ModifyRealityData()
    {
    bmap<RealityDataField, Utf8String> props = bmap<RealityDataField, Utf8String>();
    Utf8String propertyString = "\"Listable\" : true";

    RealityDataChangeRequest changeReq = RealityDataChangeRequest(m_id, propertyString);

    m_correspondance.response.clear();
    m_correspondance.req.url = changeReq.GetHttpRequestString();
    m_correspondance.req.headers = changeReq.GetRequestHeaders();
    m_correspondance.req.payload = changeReq.GetRequestPayload();

    m_correspondance.id = s_stats.LogRequest(Utf8PrintfString("Change Reality Data for user %d", m_userId));

    return WSGRequest::GetInstance().PrepareRequest(changeReq, m_correspondance.response, false, nullptr);
    }

void User::ValidateModifyRealityData(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    s_stats.InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "changedInstance") == RequestStatus::OK),
        activeUsers);
    }

CURL*  User::CreateRelationship()
    {
    RealityDataRelationshipCreateRequest relReq = RealityDataRelationshipCreateRequest(m_id, "1");

    m_correspondance.response.clear();
    m_correspondance.req.url = relReq.GetHttpRequestString();
    m_correspondance.req.headers = relReq.GetRequestHeaders();
    m_correspondance.req.payload = relReq.GetRequestPayload();

    m_correspondance.id = s_stats.LogRequest(Utf8PrintfString("Create Relationship for user %d - ID:%s", m_userId, m_id));

    return WSGRequest::GetInstance().PrepareRequest(relReq, m_correspondance.response, false, nullptr);
    }

void User::ValidateCreateRelationship(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    s_stats.InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "changedInstance") == RequestStatus::OK),
        activeUsers);

    if(m_correspondance.response.status == RequestStatus::OK)
        m_linked = true;
    }

CURL*  User::DeleteRelationship()
    {
    RealityDataRelationshipDelete relReq = RealityDataRelationshipDelete(m_id, "1");

    m_correspondance.response.clear();
    m_correspondance.req.url = relReq.GetHttpRequestString();
    m_correspondance.req.headers = relReq.GetRequestHeaders();
    m_correspondance.req.payload = relReq.GetRequestPayload();

    m_correspondance.id = s_stats.LogRequest(Utf8PrintfString("DeleteRelationship for user %d- ID:%s", m_userId, m_id));

    return WSGRequest::GetInstance().PrepareRequest(relReq, m_correspondance.response, false, nullptr);
    }

void User::ValidateDeleteRelationship(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    s_stats.InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "changedInstance") == RequestStatus::OK),
        activeUsers);

    if (m_correspondance.response.status == RequestStatus::OK)
        m_linked = false;
    }

CURL*  User::DeleteRealityData()
    {
    RealityDataDelete realityDataReq = RealityDataDelete(m_id);
    m_correspondance.response.clear();
    m_correspondance.req.url = realityDataReq.GetHttpRequestString();
    m_correspondance.req.headers = realityDataReq.GetRequestHeaders();
    m_correspondance.req.payload = realityDataReq.GetRequestPayload();

    m_correspondance.id = s_stats.LogRequest(Utf8PrintfString("Delete for user %d", m_userId));

    return WSGRequest::GetInstance().PrepareRequest(realityDataReq, m_correspondance.response, false, nullptr);
    }

void User::ValidateDeleteRealityData(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    s_stats.InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "changedInstance") == RequestStatus::OK),
        activeUsers);

    if (m_correspondance.response.status == RequestStatus::OK)
        {
        m_id = "";
        m_linked = false;
        }
    }

CURL*  User::Download()
    {
    RealityDataDocumentContentByIdRequest downloadReq = RealityDataDocumentContentByIdRequest(m_node->GetNavString());
    m_correspondance.response.clear();
    m_correspondance.req.url = downloadReq.GetHttpRequestString();
    m_correspondance.req.headers = downloadReq.GetRequestHeaders();
    m_correspondance.req.payload = downloadReq.GetRequestPayload();

    m_correspondance.id = s_stats.LogRequest(Utf8PrintfString("Download %s for user %d", m_node->GetNavString(), m_userId));

    m_file.Create(m_fileName, true);

    return WSGRequest::GetInstance().PrepareRequest(downloadReq, m_correspondance.response, false, &m_file);
    }

void User::ValidateDownload(int activeUsers)
    {
    uint64_t length;
    m_file.GetSize(length);

    s_stats.InsertStats(this,
        (length > 0),
        activeUsers);

    m_file.Close();
    m_fileName.BeDeleteFile();
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* automatically call the proper validation function for the last executed operation
//+---------------+---------------+---------------+---------------+---------------+------*/
void User::ValidatePrevious(int activeUsers)
    {
    if(m_correspondance.response.curlCode != CURLE_OK)
        {
        s_stats.InsertStats(this, false, activeUsers);
        return;
        }
    switch (m_currentOperation)
        {
        case OperationType::DETAILS:
            return ValidateDetails(activeUsers);
        case OperationType::AZURE_ADDRESS:
            return ValidateAzureAddress(activeUsers);
        case OperationType::NAVNODE:
            return ValidateNavNode(activeUsers);
        case OperationType::MODIFY_REALITYDATA:
            return ValidateModifyRealityData(activeUsers);
        case OperationType::CREATE_RELATIONSHIP:
            return ValidateCreateRelationship(activeUsers);
        case OperationType::DELETE_RELATIONSHIP:
            return ValidateDeleteRelationship(activeUsers);
        case OperationType::DELETE_REALITYDATA:
            return ValidateDeleteRealityData(activeUsers);
        case OperationType::LIST_REALITYDATA:
            return ValidateListRealityData(activeUsers);
        case OperationType::CREATE_REALITYDATA:
            return ValidateCreateRealityData(activeUsers);
        case OperationType::ENTERPRISE_STAT:
            return ValidateEnterpriseStat(activeUsers);
        case OperationType::DOWNLOAD:
            return ValidateDownload(activeUsers);
        }
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void ShowUsage()
    {
    std::cout << "Usage: RDSLoadTest.exe -u:[users]" << std::endl << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help                                      Show this help message and exit" << std::endl;
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
    for(int i = 0; i < users.size(); i++)
        delete users[i];
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
User::~User()
    {
    delete m_node;
    m_node=nullptr;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
int main(int argc, char* argv[])
    {
    // Deactivate target requests per hour (run to max)
    s_targetRequestsPerHour = 0;

    SetConsoleTitle("RDS Load Test");

    if(argc < 2)
        {
        ShowUsage();
        return 0;
        }
    
    char* substringPosition;
    int userCount = 0;
    bool trickle = false;
    Utf8String path = "";

    //parse command line arguments
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

            if (substringPosition == 0)
                {
                ShowUsage();
                return 0;
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
                ShowUsage();
                return 0;
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
    
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(s_startTime);

    UserManager wo = UserManager();
    wo.m_userCount = userCount;

    if(!trickle)
        {
        for(int i = 0; i < userCount; i++)
            {
            wo.users.push_back(new User(i));
            }
        }
    else
        {
        wo.users.push_back(new User(0)); //start with one
            for (int i = 1; i < userCount; i++)
            {
            s_innactiveUsers.push_back(new User(i)); //feed the rest to the Dispatcher
            }
        }

    std::thread dispatch (Dispatch, &wo);
    std::thread terminate (Terminate);

    RealityDataService::SetServerComponents("qa-realitydataservices-eus.cloudapp.net", "2.4", "S3MXECPlugin--Server", "S3MX");

    wo.Perform();
    std::cout << "---EXIT---" << std::endl;
    s_keepRunning = false;
    terminate.join();
    dispatch.join();
    s_stats.WriteToFile(userCount, path);
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
        users[i]->DoNext(this);
        }
    
    int still_running; /* keep number of running handles */
    int repeats = 0;
    bool wrapUp = true;

    std::cout << "launching requests, it may take a few seconds to receive responses and display results" << std::endl;

    curl_multi_perform(m_pCurlHandle, &still_running);
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
            if (repeats > 1)
                Sleep(300); /* sleep 100 milliseconds */
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

                user->m_correspondance.response.curlCode = msg->data.result;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &(user->m_correspondance.response.responseCode));

            if (msg->msg == CURLMSG_DONE)//response received, ensure that it is valid
                user->ValidatePrevious(still_running);

            std::lock_guard<std::mutex> lock(innactiveUserMutex);

            s_innactiveUsers.push_back(user);
                
            curl_multi_remove_handle(m_pCurlHandle, msg->easy_handle);
            curl_easy_cleanup(msg->easy_handle);
            }
        
        curl_multi_perform(m_pCurlHandle, &still_running);
        if(still_running == 0)
            {
            if (s_keepRunning)//no pending requests but exit has not been queued
                {
                s_stats.PrintStats();
                std::cout << "user pool empty, repopulating" << std::endl;
                Repopulate();
                }
            else if (wrapUp)
                {
                wrapUp = false;
                std::cout << "Removing created entries..." << std::endl;
                Repopulate();
                }

            curl_multi_perform(m_pCurlHandle, &still_running);
            }
        } while (wrapUp || still_running > 0);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* when the dispatcher isn't working quickly enough, restarts all innactive users
//+---------------+---------------+---------------+---------------+---------------+------*/
void UserManager::Repopulate()
    {
    std::lock_guard<std::mutex> lock(innactiveUserMutex);
    size_t innactiveUserCount = s_innactiveUsers.size();

    for (size_t i = 0; i < innactiveUserCount; i++)
        {
        s_innactiveUsers[i]->DoNext(this);
        }
    s_innactiveUsers.clear();
    }

void UserManager::SetupCurl(CURL* curl, User* user)
    {
    curl_easy_setopt(curl, CURLOPT_PRIVATE, user);
    curl_multi_add_handle((CURLM*)m_pCurlHandle, curl);
    s_rps.AddRequest(user->m_currentOperation, user->m_start);
    }