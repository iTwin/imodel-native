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

#include <Bentley/BeFile.h>
#include <RealityPlatform/RealityConversionTools.h>
#include <CCApi/CCPublic.h>

#include "3CSLoadTest.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

std::mutex innactiveUserMutex;
std::mutex statMutex;
std::mutex rpsMutex;
std::mutex guidMutex;

static std::queue<Utf8String> s_availableGuids = std::queue<Utf8String>();
static std::queue<User*> s_innactiveUsers = std::queue<User*>();
static RPS s_rps = RPS();
static bool s_keepRunning = true;
static Stats s_stats = Stats();
static const Utf8String s_server("https://qa-connect-contextcapture.bentley.com/");
static bvector<Utf8String> s_guidparts = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U" };

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Francis Boily                   09/2015
//* writes the body of a curl reponse to a Utf8String
//+---------------+---------------+---------------+---------------+---------------+------*/
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
    ((Utf8String*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String FullInfo::LogError() const
    {
    return Utf8PrintfString("Request:\n%s\n%s\n%s\nReponse:\n%lu\t%d\n%s", req.url, req.headers, req.payload, response.responseCode, response.curlCode, response.body);
    }

void FullInfo::Clear()
    {
    req.url.clear();
    req.headers.clear();
    req.payload.clear();
    response.clear();
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* request per second class
//* for each operation type, keep a cout of how many requests were sent at any given second
//+---------------+---------------+---------------+---------------+---------------+------*/
RPS::RPS():requestLog(bmap<OperationType, bmap<int64_t, int>>())
    {
    requestLog.Insert(OperationType::LIST_PROJECT, bmap<int64_t, int>());
    requestLog.Insert(OperationType::ADD_PROJECT, bmap<int64_t, int>());
    requestLog.Insert(OperationType::DELETE_PROJECT, bmap<int64_t, int>());
    requestLog.Insert(OperationType::GET_PROJECT, bmap<int64_t, int>());
    requestLog.Insert(OperationType::SAS_URI, bmap<int64_t, int>());
    requestLog.Insert(OperationType::LIST_CLUSTERS, bmap<int64_t, int>());
    requestLog.Insert(OperationType::CREATE_JOB, bmap<int64_t, int>());
    requestLog.Insert(OperationType::SUBMIT_JOB, bmap<int64_t, int>());
    requestLog.Insert(OperationType::DELETE_JOB, bmap<int64_t, int>());
    requestLog.Insert(OperationType::GET_JOBS, bmap<int64_t, int>());
    requestLog.Insert(OperationType::GET_JOB, bmap<int64_t, int>());
    //requestLog.Insert(OperationType::JOB_RESULT, bmap<int64_t, int>());
    requestLog.Insert(OperationType::CANCEL_JOB, bmap<int64_t, int>());
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

    int amount = 0;
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
    User* user = s_innactiveUsers.front();
    s_innactiveUsers.pop();
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
    opStats.Insert(OperationType::LIST_PROJECT, new Stat());
    opStats.Insert(OperationType::ADD_PROJECT, new Stat());
    opStats.Insert(OperationType::DELETE_PROJECT, new Stat());
    opStats.Insert(OperationType::GET_PROJECT, new Stat());
    opStats.Insert(OperationType::SAS_URI, new Stat());
    opStats.Insert(OperationType::LIST_CLUSTERS, new Stat());
    opStats.Insert(OperationType::CREATE_JOB, new Stat());
    opStats.Insert(OperationType::SUBMIT_JOB, new Stat());
    opStats.Insert(OperationType::DELETE_JOB, new Stat());
    opStats.Insert(OperationType::GET_JOBS, new Stat());
    opStats.Insert(OperationType::GET_JOB, new Stat());
    //opStats.Insert(OperationType::JOB_RESULT, new Stat());
    opStats.Insert(OperationType::CANCEL_JOB, new Stat());

    errors = bmap<OperationType, bvector<Utf8String>>();
    errors.Insert(OperationType::LIST_PROJECT, bvector<Utf8String>());
    errors.Insert(OperationType::ADD_PROJECT, bvector<Utf8String>());
    errors.Insert(OperationType::DELETE_PROJECT, bvector<Utf8String>());
    errors.Insert(OperationType::GET_PROJECT, bvector<Utf8String>());
    errors.Insert(OperationType::SAS_URI, bvector<Utf8String>());
    errors.Insert(OperationType::LIST_CLUSTERS, bvector<Utf8String>());
    errors.Insert(OperationType::CREATE_JOB, bvector<Utf8String>());
    errors.Insert(OperationType::SUBMIT_JOB, bvector<Utf8String>());
    errors.Insert(OperationType::DELETE_JOB, bvector<Utf8String>());
    errors.Insert(OperationType::GET_JOBS, bvector<Utf8String>());
    errors.Insert(OperationType::GET_JOB, bvector<Utf8String>());
    //errors.Insert(OperationType::JOB_RESULT, bvector<Utf8String>());
    errors.Insert(OperationType::CANCEL_JOB, bvector<Utf8String>());
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void Stats::InsertStats(const User* user, bool success, int activeUsers)
    {
    std::lock_guard<std::mutex> lock(statMutex);
    m_activeUsers = activeUsers;
    int64_t currentTime;
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);
    opStats[user->m_currentOperation]->Update(success, currentTime - user->m_start);
    if(!success)
        errors[user->m_currentOperation].push_back(user->m_correspondance.LogError());
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

    std::cout << "Type        Success    Failure   minTime   maxTime   avgTime  requests/second" << std::endl;
                                                                       
    std::cout << Utf8PrintfString("List project %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_PROJECT]->success, opStats[OperationType::LIST_PROJECT]->failure, (int)opStats[OperationType::LIST_PROJECT]->minTime, (int)opStats[OperationType::LIST_PROJECT]->maxTime, (int)opStats[OperationType::LIST_PROJECT]->avgTime, s_rps.GetRPS(OperationType::LIST_PROJECT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Add project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::ADD_PROJECT]->success, opStats[OperationType::ADD_PROJECT]->failure, (int)opStats[OperationType::ADD_PROJECT]->minTime, (int)opStats[OperationType::ADD_PROJECT]->maxTime, (int)opStats[OperationType::ADD_PROJECT]->avgTime, s_rps.GetRPS(OperationType::ADD_PROJECT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Del project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_PROJECT]->success, opStats[OperationType::DELETE_PROJECT]->failure, (int)opStats[OperationType::DELETE_PROJECT]->minTime, (int)opStats[OperationType::DELETE_PROJECT]->maxTime, (int)opStats[OperationType::DELETE_PROJECT]->avgTime, s_rps.GetRPS(OperationType::DELETE_PROJECT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Get project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_PROJECT]->success, opStats[OperationType::GET_PROJECT]->failure, (int)opStats[OperationType::GET_PROJECT]->minTime, (int)opStats[OperationType::GET_PROJECT]->maxTime, (int)opStats[OperationType::GET_PROJECT]->avgTime, s_rps.GetRPS(OperationType::GET_PROJECT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("SAS URI      %6d %10d %9d %10d %9d        %f", opStats[OperationType::SAS_URI]->success, opStats[OperationType::SAS_URI]->failure, (int)opStats[OperationType::SAS_URI]->minTime, (int)opStats[OperationType::SAS_URI]->maxTime, (int)opStats[OperationType::SAS_URI]->avgTime, s_rps.GetRPS(OperationType::SAS_URI, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("List Cluster %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_CLUSTERS]->success, opStats[OperationType::LIST_CLUSTERS]->failure, (int)opStats[OperationType::LIST_CLUSTERS]->minTime, (int)opStats[OperationType::LIST_CLUSTERS]->maxTime, (int)opStats[OperationType::LIST_CLUSTERS]->avgTime, s_rps.GetRPS(OperationType::LIST_CLUSTERS, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Create Job   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CREATE_JOB]->success, opStats[OperationType::CREATE_JOB]->failure, (int)opStats[OperationType::CREATE_JOB]->minTime, (int)opStats[OperationType::CREATE_JOB]->maxTime, (int)opStats[OperationType::CREATE_JOB]->avgTime, s_rps.GetRPS(OperationType::CREATE_JOB, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Add Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::SUBMIT_JOB]->success, opStats[OperationType::SUBMIT_JOB]->failure, (int)opStats[OperationType::SUBMIT_JOB]->minTime, (int)opStats[OperationType::SUBMIT_JOB]->maxTime, (int)opStats[OperationType::SUBMIT_JOB]->avgTime, s_rps.GetRPS(OperationType::SUBMIT_JOB, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Del Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_JOB]->success, opStats[OperationType::DELETE_JOB]->failure, (int)opStats[OperationType::DELETE_JOB]->minTime, (int)opStats[OperationType::DELETE_JOB]->maxTime, (int)opStats[OperationType::DELETE_JOB]->avgTime, s_rps.GetRPS(OperationType::DELETE_JOB, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Get Jobs     %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_JOBS]->success, opStats[OperationType::GET_JOBS]->failure, (int)opStats[OperationType::GET_JOBS]->minTime, (int)opStats[OperationType::GET_JOBS]->maxTime, (int)opStats[OperationType::GET_JOBS]->avgTime, s_rps.GetRPS(OperationType::GET_JOBS, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Get Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_JOB]->success, opStats[OperationType::GET_JOB]->failure, (int)opStats[OperationType::GET_JOB]->minTime, (int)opStats[OperationType::GET_JOB]->maxTime, (int)opStats[OperationType::GET_JOB]->avgTime, s_rps.GetRPS(OperationType::GET_JOB, currentTime)) << std::endl;
    //std::cout << Utf8PrintfString("Job Result   %6d %10d %9d %10d %9d        %f", opStats[OperationType::JOB_RESULT]->success, opStats[OperationType::JOB_RESULT]->failure, (int)opStats[OperationType::JOB_RESULT]->minTime, (int)opStats[OperationType::JOB_RESULT]->maxTime, (int)opStats[OperationType::JOB_RESULT]->avgTime, s_rps.GetRPS(OperationType::JOB_RESULT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Job Cancel   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CANCEL_JOB]->success, opStats[OperationType::CANCEL_JOB]->failure, (int)opStats[OperationType::CANCEL_JOB]->minTime, (int)opStats[OperationType::CANCEL_JOB]->maxTime, (int)opStats[OperationType::CANCEL_JOB]->avgTime, s_rps.GetRPS(OperationType::CANCEL_JOB, currentTime)) << std::endl;

    std::cout << "active users: " << m_activeUsers << std::endl << std::endl;

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
    file << asctime(localtime(&generatedFileName)) << std::endl;
    file << "Type        Success    Failure   minTime   maxTime   avgTime" << std::endl;
    
    int64_t currentTime;
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);

    file << Utf8PrintfString("List project %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_PROJECT]->success, opStats[OperationType::LIST_PROJECT]->failure, (int)opStats[OperationType::LIST_PROJECT]->minTime, (int)opStats[OperationType::LIST_PROJECT]->maxTime, (int)opStats[OperationType::LIST_PROJECT]->avgTime, s_rps.GetRPS(OperationType::LIST_PROJECT, currentTime)) << std::endl;
    file << Utf8PrintfString("Add project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::ADD_PROJECT]->success, opStats[OperationType::ADD_PROJECT]->failure, (int)opStats[OperationType::ADD_PROJECT]->minTime, (int)opStats[OperationType::ADD_PROJECT]->maxTime, (int)opStats[OperationType::ADD_PROJECT]->avgTime, s_rps.GetRPS(OperationType::ADD_PROJECT, currentTime)) << std::endl;
    file << Utf8PrintfString("Del project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_PROJECT]->success, opStats[OperationType::DELETE_PROJECT]->failure, (int)opStats[OperationType::DELETE_PROJECT]->minTime, (int)opStats[OperationType::DELETE_PROJECT]->maxTime, (int)opStats[OperationType::DELETE_PROJECT]->avgTime, s_rps.GetRPS(OperationType::DELETE_PROJECT, currentTime)) << std::endl;
    file << Utf8PrintfString("Get project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_PROJECT]->success, opStats[OperationType::GET_PROJECT]->failure, (int)opStats[OperationType::GET_PROJECT]->minTime, (int)opStats[OperationType::GET_PROJECT]->maxTime, (int)opStats[OperationType::GET_PROJECT]->avgTime, s_rps.GetRPS(OperationType::GET_PROJECT, currentTime)) << std::endl;
    file << Utf8PrintfString("SAS URI      %6d %10d %9d %10d %9d        %f", opStats[OperationType::SAS_URI]->success, opStats[OperationType::SAS_URI]->failure, (int)opStats[OperationType::SAS_URI]->minTime, (int)opStats[OperationType::SAS_URI]->maxTime, (int)opStats[OperationType::SAS_URI]->avgTime, s_rps.GetRPS(OperationType::SAS_URI, currentTime)) << std::endl;
    file << Utf8PrintfString("List Cluster %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_CLUSTERS]->success, opStats[OperationType::LIST_CLUSTERS]->failure, (int)opStats[OperationType::LIST_CLUSTERS]->minTime, (int)opStats[OperationType::LIST_CLUSTERS]->maxTime, (int)opStats[OperationType::LIST_CLUSTERS]->avgTime, s_rps.GetRPS(OperationType::LIST_CLUSTERS, currentTime)) << std::endl;
    file << Utf8PrintfString("Create Job   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CREATE_JOB]->success, opStats[OperationType::CREATE_JOB]->failure, (int)opStats[OperationType::CREATE_JOB]->minTime, (int)opStats[OperationType::CREATE_JOB]->maxTime, (int)opStats[OperationType::CREATE_JOB]->avgTime, s_rps.GetRPS(OperationType::CREATE_JOB, currentTime)) << std::endl;
    file << Utf8PrintfString("Add Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::SUBMIT_JOB]->success, opStats[OperationType::SUBMIT_JOB]->failure, (int)opStats[OperationType::SUBMIT_JOB]->minTime, (int)opStats[OperationType::SUBMIT_JOB]->maxTime, (int)opStats[OperationType::SUBMIT_JOB]->avgTime, s_rps.GetRPS(OperationType::SUBMIT_JOB, currentTime)) << std::endl;
    file << Utf8PrintfString("Del Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_JOB]->success, opStats[OperationType::DELETE_JOB]->failure, (int)opStats[OperationType::DELETE_JOB]->minTime, (int)opStats[OperationType::DELETE_JOB]->maxTime, (int)opStats[OperationType::DELETE_JOB]->avgTime, s_rps.GetRPS(OperationType::DELETE_JOB, currentTime)) << std::endl;
    file << Utf8PrintfString("Get Jobs     %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_JOBS]->success, opStats[OperationType::GET_JOBS]->failure, (int)opStats[OperationType::GET_JOBS]->minTime, (int)opStats[OperationType::GET_JOBS]->maxTime, (int)opStats[OperationType::GET_JOBS]->avgTime, s_rps.GetRPS(OperationType::GET_JOBS, currentTime)) << std::endl;
    file << Utf8PrintfString("Get Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_JOB]->success, opStats[OperationType::GET_JOB]->failure, (int)opStats[OperationType::GET_JOB]->minTime, (int)opStats[OperationType::GET_JOB]->maxTime, (int)opStats[OperationType::GET_JOB]->avgTime, s_rps.GetRPS(OperationType::GET_JOB, currentTime)) << std::endl;
    //file << Utf8PrintfString("Job Result   %6d %10d %9d %10d %9d        %f", opStats[OperationType::JOB_RESULT]->success, opStats[OperationType::JOB_RESULT]->failure, (int)opStats[OperationType::JOB_RESULT]->minTime, (int)opStats[OperationType::JOB_RESULT]->maxTime, (int)opStats[OperationType::JOB_RESULT]->avgTime, s_rps.GetRPS(OperationType::JOB_RESULT, currentTime)) << std::endl;
    file << Utf8PrintfString("Job Cancel   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CANCEL_JOB]->success, opStats[OperationType::CANCEL_JOB]->failure, (int)opStats[OperationType::CANCEL_JOB]->minTime, (int)opStats[OperationType::CANCEL_JOB]->maxTime, (int)opStats[OperationType::CANCEL_JOB]->avgTime, s_rps.GetRPS(OperationType::CANCEL_JOB, currentTime)) << std::endl;

    file << std::endl << std::endl << "operation list:" << std::endl;

    for (Utf8String op : opLog)
        file << op << std::endl;

    file << std::endl << std::endl << "error list:" << std::endl;
    
    file << "List Projects:" << std::endl;
    for (Utf8String error : errors[OperationType::LIST_PROJECT])
        file << error << std::endl;

    file << "Add Project:" << std::endl;
    for (Utf8String error : errors[OperationType::ADD_PROJECT])
        file << error << std::endl;

    file << "Delete Project:" << std::endl;
    for (Utf8String error : errors[OperationType::DELETE_PROJECT])
        file << error << std::endl;

    file << "Get Project:" << std::endl;
    for (Utf8String error : errors[OperationType::GET_PROJECT])
        file << error << std::endl;

    file << "SAS Uri:" << std::endl;
    for (Utf8String error : errors[OperationType::SAS_URI])
        file << error << std::endl;

    file << "List Clusters:" << std::endl;
    for (Utf8String error : errors[OperationType::LIST_CLUSTERS])
        file << error << std::endl;

    file << "Create Job:" << std::endl;
    for (Utf8String error : errors[OperationType::CREATE_JOB])
        file << error << std::endl;

    file << "Add Job:" << std::endl;
    for (Utf8String error : errors[OperationType::SUBMIT_JOB])
        file << error << std::endl;

    file << "Delete Job:" << std::endl;
    for (Utf8String error : errors[OperationType::DELETE_JOB])
        file << error << std::endl;

    file << "Get Jobs:" << std::endl;
    for (Utf8String error : errors[OperationType::GET_JOBS])
        file << error << std::endl;

    file << "Get Job by Id:" << std::endl;
    for (Utf8String error : errors[OperationType::GET_JOB])
        file << error << std::endl;

    /*file << "Job Result:" << std::endl;
    for (Utf8String error : errors[OperationType::JOB_RESULT])
        file << error << std::endl;*/

    file << "Cancel Job:" << std::endl;
    for (Utf8String error : errors[OperationType::CANCEL_JOB])
        file << error << std::endl;

    file.close();
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
User::User(int id, Utf8String token) :
    m_currentOperation(OperationType::LIST_PROJECT), m_userId(id), m_token(token),
    m_submitted(false)
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
    if(m_id.empty() && !m_jobId.empty())
        m_currentOperation = OperationType::DELETE_JOB;
    
    if (m_currentOperation == OperationType::DELETE_PROJECT)
        {
        if(m_id.empty())
            m_currentOperation = OperationType::ADD_PROJECT;
        else
            curl = DeleteProject();
        }
    else if (m_currentOperation == OperationType::GET_PROJECT)
        {
        if (m_id.empty())
            m_currentOperation = OperationType::ADD_PROJECT;
        else
            curl = GetProject();
        }
    else if (m_currentOperation == OperationType::SAS_URI)
        {
        if (m_id.empty())
            m_currentOperation = OperationType::ADD_PROJECT;
        else
            curl = SASUri();
        }
    else if (m_currentOperation == OperationType::SUBMIT_JOB)
        {
        if (m_jobId.empty())
            m_currentOperation = OperationType::CREATE_JOB;
        else if (m_submitted)
            {
            m_currentOperation = OperationType::CANCEL_JOB;
            curl = CancelJob();
            }
        else
            curl = SubmitJob();
        }
    else if (m_currentOperation == OperationType::DELETE_JOB)
        {
        if (m_jobId.empty())
            m_currentOperation = OperationType::CREATE_JOB;
        else
            curl = DeleteJob();
        }
    else if (m_currentOperation == OperationType::GET_JOB)
        {
        if (m_jobId.empty())
            m_currentOperation = OperationType::CREATE_JOB;
        else
            curl = GetJobById();
        }
    /*else if (m_currentOperation == OperationType::JOB_RESULT)
        {
        if (m_jobId.empty())
            m_currentOperation = OperationType::CREATE_JOB;
        else
            curl = GetJobResult();
        }*/
    else if (m_currentOperation == OperationType::CANCEL_JOB)
        {
        if (m_jobId.empty())
            m_currentOperation = OperationType::CREATE_JOB;
        else if (!m_submitted)
            {
            m_currentOperation = OperationType::SUBMIT_JOB;
            curl = SubmitJob();
            }
        else
            curl = CancelJob();
        }
    else if (m_currentOperation == OperationType::LIST_CLUSTERS)
        {
        if (m_id.empty())
            m_currentOperation = OperationType::ADD_PROJECT;
        else
            curl = ListClusters();
        }
    
    if (m_currentOperation == OperationType::CREATE_JOB)
        {
        std::lock_guard<std::mutex> lock(guidMutex);
        if (m_id.empty())
            m_currentOperation = OperationType::ADD_PROJECT;
        else if (!m_jobId.empty())
            {
            m_currentOperation = OperationType::DELETE_JOB;
            curl = DeleteJob();
            }
        else if (s_availableGuids.size() == 0)
            {
            m_currentOperation = OperationType::DELETE_PROJECT;
            curl = DeleteProject();
            }
        else
            {
            curl = CreateJob(s_availableGuids.front());
            s_availableGuids.pop();
            }
        }
    
    if (m_currentOperation == OperationType::ADD_PROJECT)
        {
        if (!m_id.empty())
            {
            m_currentOperation = OperationType::DELETE_PROJECT;
            curl = DeleteProject();
            }
        else
            curl = AddProject();
        }
    else if(m_currentOperation == OperationType::LIST_PROJECT)
        {
        curl = ListProject();
        }
    else if (m_currentOperation == OperationType::GET_JOBS)
        {
        curl = GetJobs();
        }

    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(m_start);

    if(curl != nullptr)
        owner->SetupCurl(curl, this);
    else
        {
        std::lock_guard<std::mutex> lock(innactiveUserMutex);
        s_innactiveUsers.push(this);
        }
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* prepares next request, based on most recent action
//* can be called blindly from outside
//+---------------+---------------+---------------+---------------+---------------+------*/
void User::WrapUp(UserManager* owner)
    {
    CURL* curl = nullptr;
    
    if (!m_jobId.empty())
        {
        m_currentOperation = OperationType::DELETE_JOB;
        curl = DeleteJob();
        }
    else if (!m_id.empty())
        {
        m_currentOperation = OperationType::DELETE_PROJECT;
        curl = DeleteProject();
        }

    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(m_start);
    if (curl != nullptr)
        owner->SetupCurl(curl, this);
    }

CURL* User::ListProject()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append("api/v1/projects");
    m_correspondance.req.type = GET;

    return PrepareRequest();
    }

CURL* User::AddProject()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append("api/v1/projects");
    m_correspondance.req.type = POST;
    m_correspondance.req.payload = "{\"region\": \"eus\", \"name\":\"something\"}";

    return PrepareRequest();
    }

CURL* User::DeleteProject()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/projects/%s", m_id));
    m_correspondance.req.type = DEL;

    s_availableGuids.push(m_id);

    m_id.clear();

    return PrepareRequest();
    }

CURL* User::GetProject()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/projects/%s", m_id));
    m_correspondance.req.type = GET;

    return PrepareRequest();
    }

CURL* User::SASUri()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/projects/%s/sas-uri", m_id));
    m_correspondance.req.type = GET;

    return PrepareRequest();
    }

CURL* User::ListClusters()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/projects/%s/clusters", m_id));
    m_correspondance.req.type = GET;

    return PrepareRequest();
    }

CURL* User::CreateJob(Utf8String outputGuid)
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append("api/v1/jobs");
    m_correspondance.req.type = POST;
    Utf8String body =   "{"
                            "\"projectName\" : \"load test job\","
                            "\"jobType\" : \"Photos2Mesh\","
                            "\"projectId\": \"%s\","
                            "\"inputs\" : {"
                                "\"storage\":{"
                                    "\"storageType\":\"Rds\","
                                    "\"path\" : \"0c907846-aaf3-45c7-a183-73937c90b0f6\""
                                "}"
                            "},"
                            "\"submissionDetails\":{"
                                "\"hostname\":\"EMEA84158\","
                                "\"email\" : \"Emmanuel.Pot@bentley.com\","
                                "\"organizationId\" : \"e82a584b-9fae-409f-9581-fd154f7b9ef9\","
                                "\"time\" : \"20170427T134744\""
                            "},"
                            "\"status\":\"Pending\","
                            "\"priority\" : 0,"
                            "\"settings\" : {"
                                "\"outputs\":["
                                    "{"
                                        "\"format\":\"3MX\", "
                                        "\"storages\" : ["
                                            "{"
                                                "\"storageType\":\"Rds\", "
                                                "\"path\" : \"%s\""
                                            "}"
                                        "]"
                                    "}"
                                "]," 
                            "\"publicWebGL\" : true, "
                            "\"meshQuality\" : "
                            "\"medium\""
                            "}, "
                            "\"resultPublisher\" : {"
                                "\"resultPublisherType\":\"CCCS\","
                                "\"path\" : \"https://qa-connect-contextcapture.bentley.com/api/v1/job-result\""
                            "}"
                        "}";
    
    m_correspondance.req.payload = Utf8PrintfString(body.c_str(), m_id, outputGuid);
    
    return PrepareRequest();
    }

CURL* User::SubmitJob()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/jobs/%s/submit", m_jobId));
    m_correspondance.req.type = POST;
    
    m_submitted = true;

    return PrepareRequest();
    }

CURL* User::DeleteJob()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/jobs/%s", m_jobId));
    m_correspondance.req.type = DEL;

    m_submitted = false;

    m_jobId.clear();

    return PrepareRequest();
    }   

CURL* User::GetJobs()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append("api/v1/jobs");
    m_correspondance.req.type = GET;

    return PrepareRequest();
    }

CURL* User::GetJobById()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/jobs/%s", m_jobId));
    m_correspondance.req.type = GET;

    return PrepareRequest();
    }

/*CURL* User::GetJobResult()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append("api/v1/job-result");
    m_correspondance.req.type = POST;

    return PrepareRequest();
    }*/

CURL* User::CancelJob()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/jobs/%s/cancel", m_jobId));
    m_correspondance.req.type = POST;

    return PrepareRequest();
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* automatically call the proper validation function for the last executed operation
//+---------------+---------------+---------------+---------------+---------------+------*/
void User::ValidatePrevious(int activeUsers)
    {
    switch (m_currentOperation)
        {
        case OperationType::ADD_PROJECT:
            return ValidateAddProject(activeUsers);
        case OperationType::CREATE_JOB:
            return ValidateCreateJob(activeUsers);
        case OperationType::CANCEL_JOB:
            return ValidateCancelJob(activeUsers);
        default:
            {
            if(m_correspondance.response.curlCode != CURLE_OK || m_correspondance.response.responseCode > 399)
                {
                s_stats.InsertStats(this, false, activeUsers);
                return;
                }
            else
                s_stats.InsertStats(this, (m_correspondance.response.curlCode == CURLE_OK), activeUsers);
            }
        }
    }

void User::ValidateAddProject(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    bool success = false;

    if (m_correspondance.response.ValidateJSONResponse(instances, "id") == RequestStatus::OK)
        {
        success = true;
        m_id = instances["id"].asString();
        }
    
    s_stats.InsertStats(this, success, activeUsers);
    }

void User::ValidateCreateJob(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    bool success = false;

    if (m_correspondance.response.ValidateJSONResponse(instances, "id") == RequestStatus::OK)
        {
        success = true;
        m_jobId = instances["id"].asString();
        m_submitted = false;
        }

    s_stats.InsertStats(this, success, activeUsers);
    }

void User::ValidateCancelJob(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    bool success = false;

    if (m_correspondance.response.responseCode < 399 || m_correspondance.response.body.ContainsI("Job does not seem to be pending nor running"))
        {
        success = true;
        m_submitted = false;
        }

    s_stats.InsertStats(this, success, activeUsers);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void ShowUsage()
    {
    std::cout << "Usage: 3CSLoadTest.exe -u:[users]" << std::endl << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help              Show this help message and exit" << std::endl;
    std::cout << "  -u, --users             Number of users" << std::endl;
    std::cout << "  -t, --trickle           optional, add this argument to avoid spawing all users at once" << std::endl;
    std::cout << "  -p, --path              optional, specifiy a path to out the log file to" << std::endl;
    std::cout << "                          any text added after the last backslash will be added to the file name" << std::endl;
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
int main(int argc, char* argv[])
    {
    SetConsoleTitle("3CS Load Test");

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
            substringPosition++;
            Utf8String userString = Utf8String(substringPosition);
            uint64_t users;
            BeStringUtilities::ParseUInt64(users, userString.c_str());
            userCount = (int)users;
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
    
    UserManager wo = UserManager();
    wo.m_userCount = userCount;

    CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
    CallStatus status = APIERR_SUCCESS;

    bool installed;
    status = CCApi_IsInstalled(api, &installed);
    if (!installed)
        {
        std::cout << "Connection client does not seem to be installed" << std::endl;
        return -1;
        }
    bool running = false;
    status = CCApi_IsRunning(api, &running);
    if (status != APIERR_SUCCESS || !running)
        {
        std::cout << "Connection client does not seem to be running" << std::endl;
        return -1; 
        }
    bool loggedIn = false;
    status = CCApi_IsLoggedIn(api, &loggedIn);
    if (status != APIERR_SUCCESS || !loggedIn)
        {
        std::cout << "Connection client does not seem to be logged in" << std::endl;
        return -1;
        }
    bool acceptedEula = false;
    status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
    if (status != APIERR_SUCCESS || !acceptedEula)
        {
        std::cout << "Connection client user does not seem to have accepted EULA" << std::endl;
        return -1;
        }
    bool sessionActive = false;
    status = CCApi_IsUserSessionActive(api, &sessionActive);
    if (status != APIERR_SUCCESS || !sessionActive)
        {
        std::cout << "Connection client does not seem to have an active session" << std::endl;
        return -1;
        }

    LPCWSTR relyingParty = L"https://connect-wsg20.bentley.com";//;L"https:://qa-ims.bentley.com"
    UINT32 maxTokenLength = 16384;
    LPWSTR lpwstrToken = new WCHAR[maxTokenLength];

    status = CCApi_GetSerializedDelegateSecurityToken(api, relyingParty, lpwstrToken, maxTokenLength);
    if (status != APIERR_SUCCESS)
        return -1;

    char* charToken = new char[maxTokenLength];
    wcstombs(charToken, lpwstrToken, maxTokenLength);

    Utf8String token = "Authorization: Token ";
    token.append(charToken);

    delete lpwstrToken;
    delete charToken;

    if(!trickle)
        {
        for(int i = 0; i < userCount; i++)
            {
            wo.users.push_back(new User(i, token));
            }
        }
    else
        {
        wo.users.push_back(new User(0, token)); //start with one
            for (int i = 1; i < userCount; i++)
            {
            s_innactiveUsers.push(new User(i, token)); //feed the rest to the Dispatcher
            }
        }

    std::thread dispatch (Dispatch, &wo);
    std::thread terminate (Terminate);

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
            s_innactiveUsers.push(user);
                
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
        User* user = s_innactiveUsers.front();
        s_innactiveUsers.pop();
        user->DoNext(this);
        }
    }

void UserManager::SetupCurl(CURL* curl, User* user)
    {
    curl_easy_setopt(curl, CURLOPT_PRIVATE, user);
    curl_multi_add_handle((CURLM*)m_pCurlHandle, curl);
    s_rps.AddRequest(user->m_currentOperation, user->m_start);
    }

CURL* User::PrepareRequest()
    {
    CURL* curl = curl_easy_init();
    if (nullptr == curl)
        {
        m_correspondance.response.curlCode = CURLcode::CURLE_FAILED_INIT;
        return curl;
        }

    //Adjusting headers for the POST method
    struct curl_slist *headers = NULL;

    bvector<Utf8String> wsgHeaders = m_correspondance.req.headers;
    for (Utf8String header : wsgHeaders)
        headers = curl_slist_append(headers, header.c_str());
    headers = curl_slist_append(headers, m_token.c_str());

    if (m_correspondance.req.type == requestType::POST)
        {
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, m_correspondance.req.payload.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, m_correspondance.req.payload.length());
        }
    else if (m_correspondance.req.type == requestType::DEL)
        {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, m_correspondance.req.payload.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, m_correspondance.req.payload.length());
        }

    curl_easy_setopt(curl, CURLOPT_URL, m_correspondance.req.url);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);

    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &(m_correspondance.response.header));

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(m_correspondance.response.body));

    return curl;
    }