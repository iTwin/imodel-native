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

#include "3CSLoadTest.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

std::mutex innactiveUserMutex;
std::mutex statMutex;
std::mutex rpsMutex;

static bvector<User*> s_innactiveUsers = bvector<User*>();
static RPS s_rps = RPS();
static bool s_keepRunning = true;
static Stats s_stats = Stats();
static const Utf8String s_server("https://dev-contextcapture-eus.cloudapp.net/");

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
    return Utf8PrintfString("Request %d :\n%s\n%s\n%s\nReponse:\n%lu\t%d\n%s", id, req.url, req.headers, req.payload, response.responseCode, response.curlCode, response.body);
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
RPS::RPS():requestLog(bmap<OperationType, bmap<time_t, int>>())
    {
    requestLog.Insert(OperationType::LIST_PROJECT, bmap<time_t, int>());
    requestLog.Insert(OperationType::ADD_PROJECT, bmap<time_t, int>());
    requestLog.Insert(OperationType::DELETE_PROJECT, bmap<time_t, int>());
    requestLog.Insert(OperationType::GET_PROJECT, bmap<time_t, int>());
    requestLog.Insert(OperationType::SAS_URI, bmap<time_t, int>());
    requestLog.Insert(OperationType::LIST_CLUSTERS, bmap<time_t, int>());
    requestLog.Insert(OperationType::CREATE_JOB, bmap<time_t, int>());
    requestLog.Insert(OperationType::ADD_JOB, bmap<time_t, int>());
    requestLog.Insert(OperationType::DELETE_JOB, bmap<time_t, int>());
    requestLog.Insert(OperationType::GET_JOBS, bmap<time_t, int>());
    requestLog.Insert(OperationType::GET_JOB, bmap<time_t, int>());
    requestLog.Insert(OperationType::JOB_RESULT, bmap<time_t, int>());
    requestLog.Insert(OperationType::JOB_CANCEL, bmap<time_t, int>());
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void RPS::AddRequest(OperationType type, time_t time)
    {
    std::lock_guard<std::mutex> lock(rpsMutex);
    requestLog[type][time] += 1;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
double RPS::GetRPS(OperationType type, time_t time)
    {
    bmap<time_t, int> times = requestLog[type];

    int amount = 0;
    //get the average number of requests per second, over ten seconds
    for(time_t i = (time - 12); i < (time - 2); i++)
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
    opStats.Insert(OperationType::ADD_JOB, new Stat());
    opStats.Insert(OperationType::DELETE_JOB, new Stat());
    opStats.Insert(OperationType::GET_JOBS, new Stat());
    opStats.Insert(OperationType::GET_JOB, new Stat());
    opStats.Insert(OperationType::JOB_RESULT, new Stat());
    opStats.Insert(OperationType::JOB_CANCEL, new Stat());

    errors = bmap<OperationType, bvector<Utf8String>>();
    errors.Insert(OperationType::LIST_PROJECT, bvector<Utf8String>());
    errors.Insert(OperationType::ADD_PROJECT, bvector<Utf8String>());
    errors.Insert(OperationType::DELETE_PROJECT, bvector<Utf8String>());
    errors.Insert(OperationType::GET_PROJECT, bvector<Utf8String>());
    errors.Insert(OperationType::SAS_URI, bvector<Utf8String>());
    errors.Insert(OperationType::LIST_CLUSTERS, bvector<Utf8String>());
    errors.Insert(OperationType::CREATE_JOB, bvector<Utf8String>());
    errors.Insert(OperationType::ADD_JOB, bvector<Utf8String>());
    errors.Insert(OperationType::DELETE_JOB, bvector<Utf8String>());
    errors.Insert(OperationType::GET_JOBS, bvector<Utf8String>());
    errors.Insert(OperationType::GET_JOB, bvector<Utf8String>());
    errors.Insert(OperationType::JOB_RESULT, bvector<Utf8String>());
    errors.Insert(OperationType::JOB_CANCEL, bvector<Utf8String>());
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void Stats::InsertStats(const User* user, bool success, int activeUsers)
    {
    std::lock_guard<std::mutex> lock(statMutex);
    m_activeUsers = activeUsers;
    opStats[user->m_currentOperation]->Update(success, std::time(nullptr) - user->m_start);
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
    time_t currentTime = std::time(nullptr);
    system("cls");

    std::cout << "Type        Success    Failure   minTime   maxTime   avgTime  requests/second" << std::endl;
    
    std::cout << Utf8PrintfString("List project %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_PROJECT]->success, opStats[OperationType::LIST_PROJECT]->failure, (int)opStats[OperationType::LIST_PROJECT]->minTime, (int)opStats[OperationType::LIST_PROJECT]->maxTime, (int)opStats[OperationType::LIST_PROJECT]->avgTime, s_rps.GetRPS(OperationType::LIST_PROJECT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Add project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::ADD_PROJECT]->success, opStats[OperationType::ADD_PROJECT]->failure, (int)opStats[OperationType::ADD_PROJECT]->minTime, (int)opStats[OperationType::ADD_PROJECT]->maxTime, (int)opStats[OperationType::ADD_PROJECT]->avgTime, s_rps.GetRPS(OperationType::ADD_PROJECT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Del project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_PROJECT]->success, opStats[OperationType::DELETE_PROJECT]->failure, (int)opStats[OperationType::DELETE_PROJECT]->minTime, (int)opStats[OperationType::DELETE_PROJECT]->maxTime, (int)opStats[OperationType::DELETE_PROJECT]->avgTime, s_rps.GetRPS(OperationType::DELETE_PROJECT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Get project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_PROJECT]->success, opStats[OperationType::GET_PROJECT]->failure, (int)opStats[OperationType::GET_PROJECT]->minTime, (int)opStats[OperationType::GET_PROJECT]->maxTime, (int)opStats[OperationType::GET_PROJECT]->avgTime, s_rps.GetRPS(OperationType::GET_PROJECT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("SAS URI      %6d %10d %9d %10d %9d        %f", opStats[OperationType::SAS_URI]->success, opStats[OperationType::SAS_URI]->failure, (int)opStats[OperationType::SAS_URI]->minTime, (int)opStats[OperationType::SAS_URI]->maxTime, (int)opStats[OperationType::SAS_URI]->avgTime, s_rps.GetRPS(OperationType::SAS_URI, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("List Cluster %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_CLUSTERS]->success, opStats[OperationType::LIST_CLUSTERS]->failure, (int)opStats[OperationType::LIST_CLUSTERS]->minTime, (int)opStats[OperationType::LIST_CLUSTERS]->maxTime, (int)opStats[OperationType::LIST_CLUSTERS]->avgTime, s_rps.GetRPS(OperationType::LIST_CLUSTERS, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Create Job   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CREATE_JOB]->success, opStats[OperationType::CREATE_JOB]->failure, (int)opStats[OperationType::CREATE_JOB]->minTime, (int)opStats[OperationType::CREATE_JOB]->maxTime, (int)opStats[OperationType::CREATE_JOB]->avgTime, s_rps.GetRPS(OperationType::CREATE_JOB, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Add Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::ADD_JOB]->success, opStats[OperationType::ADD_JOB]->failure, (int)opStats[OperationType::ADD_JOB]->minTime, (int)opStats[OperationType::ADD_JOB]->maxTime, (int)opStats[OperationType::ADD_JOB]->avgTime, s_rps.GetRPS(OperationType::ADD_JOB, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Del Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_JOB]->success, opStats[OperationType::DELETE_JOB]->failure, (int)opStats[OperationType::DELETE_JOB]->minTime, (int)opStats[OperationType::DELETE_JOB]->maxTime, (int)opStats[OperationType::DELETE_JOB]->avgTime, s_rps.GetRPS(OperationType::DELETE_JOB, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Get Jobs     %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_JOBS]->success, opStats[OperationType::GET_JOBS]->failure, (int)opStats[OperationType::GET_JOBS]->minTime, (int)opStats[OperationType::GET_JOBS]->maxTime, (int)opStats[OperationType::GET_JOBS]->avgTime, s_rps.GetRPS(OperationType::GET_JOBS, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Get Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_JOB]->success, opStats[OperationType::GET_JOB]->failure, (int)opStats[OperationType::GET_JOB]->minTime, (int)opStats[OperationType::GET_JOB]->maxTime, (int)opStats[OperationType::GET_JOB]->avgTime, s_rps.GetRPS(OperationType::GET_JOB, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Job Result   %6d %10d %9d %10d %9d        %f", opStats[OperationType::JOB_RESULT]->success, opStats[OperationType::JOB_RESULT]->failure, (int)opStats[OperationType::JOB_RESULT]->minTime, (int)opStats[OperationType::JOB_RESULT]->maxTime, (int)opStats[OperationType::JOB_RESULT]->avgTime, s_rps.GetRPS(OperationType::JOB_RESULT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Job Cancel   %6d %10d %9d %10d %9d        %f", opStats[OperationType::JOB_CANCEL]->success, opStats[OperationType::JOB_CANCEL]->failure, (int)opStats[OperationType::JOB_CANCEL]->minTime, (int)opStats[OperationType::JOB_CANCEL]->maxTime, (int)opStats[OperationType::JOB_CANCEL]->avgTime, s_rps.GetRPS(OperationType::JOB_CANCEL, currentTime)) << std::endl;

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
    
    time_t currentTime = std::time(nullptr);

    file << Utf8PrintfString("List project %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_PROJECT]->success, opStats[OperationType::LIST_PROJECT]->failure, (int)opStats[OperationType::LIST_PROJECT]->minTime, (int)opStats[OperationType::LIST_PROJECT]->maxTime, (int)opStats[OperationType::LIST_PROJECT]->avgTime, s_rps.GetRPS(OperationType::LIST_PROJECT, currentTime)) << std::endl;
    file << Utf8PrintfString("Add project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::ADD_PROJECT]->success, opStats[OperationType::ADD_PROJECT]->failure, (int)opStats[OperationType::ADD_PROJECT]->minTime, (int)opStats[OperationType::ADD_PROJECT]->maxTime, (int)opStats[OperationType::ADD_PROJECT]->avgTime, s_rps.GetRPS(OperationType::ADD_PROJECT, currentTime)) << std::endl;
    file << Utf8PrintfString("Del project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_PROJECT]->success, opStats[OperationType::DELETE_PROJECT]->failure, (int)opStats[OperationType::DELETE_PROJECT]->minTime, (int)opStats[OperationType::DELETE_PROJECT]->maxTime, (int)opStats[OperationType::DELETE_PROJECT]->avgTime, s_rps.GetRPS(OperationType::DELETE_PROJECT, currentTime)) << std::endl;
    file << Utf8PrintfString("Get project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_PROJECT]->success, opStats[OperationType::GET_PROJECT]->failure, (int)opStats[OperationType::GET_PROJECT]->minTime, (int)opStats[OperationType::GET_PROJECT]->maxTime, (int)opStats[OperationType::GET_PROJECT]->avgTime, s_rps.GetRPS(OperationType::GET_PROJECT, currentTime)) << std::endl;
    file << Utf8PrintfString("SAS URI      %6d %10d %9d %10d %9d        %f", opStats[OperationType::SAS_URI]->success, opStats[OperationType::SAS_URI]->failure, (int)opStats[OperationType::SAS_URI]->minTime, (int)opStats[OperationType::SAS_URI]->maxTime, (int)opStats[OperationType::SAS_URI]->avgTime, s_rps.GetRPS(OperationType::SAS_URI, currentTime)) << std::endl;
    file << Utf8PrintfString("List Cluster %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_CLUSTERS]->success, opStats[OperationType::LIST_CLUSTERS]->failure, (int)opStats[OperationType::LIST_CLUSTERS]->minTime, (int)opStats[OperationType::LIST_CLUSTERS]->maxTime, (int)opStats[OperationType::LIST_CLUSTERS]->avgTime, s_rps.GetRPS(OperationType::LIST_CLUSTERS, currentTime)) << std::endl;
    file << Utf8PrintfString("Create Job   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CREATE_JOB]->success, opStats[OperationType::CREATE_JOB]->failure, (int)opStats[OperationType::CREATE_JOB]->minTime, (int)opStats[OperationType::CREATE_JOB]->maxTime, (int)opStats[OperationType::CREATE_JOB]->avgTime, s_rps.GetRPS(OperationType::CREATE_JOB, currentTime)) << std::endl;
    file << Utf8PrintfString("Add Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::ADD_JOB]->success, opStats[OperationType::ADD_JOB]->failure, (int)opStats[OperationType::ADD_JOB]->minTime, (int)opStats[OperationType::ADD_JOB]->maxTime, (int)opStats[OperationType::ADD_JOB]->avgTime, s_rps.GetRPS(OperationType::ADD_JOB, currentTime)) << std::endl;
    file << Utf8PrintfString("Del Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_JOB]->success, opStats[OperationType::DELETE_JOB]->failure, (int)opStats[OperationType::DELETE_JOB]->minTime, (int)opStats[OperationType::DELETE_JOB]->maxTime, (int)opStats[OperationType::DELETE_JOB]->avgTime, s_rps.GetRPS(OperationType::DELETE_JOB, currentTime)) << std::endl;
    file << Utf8PrintfString("Get Jobs     %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_JOBS]->success, opStats[OperationType::GET_JOBS]->failure, (int)opStats[OperationType::GET_JOBS]->minTime, (int)opStats[OperationType::GET_JOBS]->maxTime, (int)opStats[OperationType::GET_JOBS]->avgTime, s_rps.GetRPS(OperationType::GET_JOBS, currentTime)) << std::endl;
    file << Utf8PrintfString("Get Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_JOB]->success, opStats[OperationType::GET_JOB]->failure, (int)opStats[OperationType::GET_JOB]->minTime, (int)opStats[OperationType::GET_JOB]->maxTime, (int)opStats[OperationType::GET_JOB]->avgTime, s_rps.GetRPS(OperationType::GET_JOB, currentTime)) << std::endl;
    file << Utf8PrintfString("Job Result   %6d %10d %9d %10d %9d        %f", opStats[OperationType::JOB_RESULT]->success, opStats[OperationType::JOB_RESULT]->failure, (int)opStats[OperationType::JOB_RESULT]->minTime, (int)opStats[OperationType::JOB_RESULT]->maxTime, (int)opStats[OperationType::JOB_RESULT]->avgTime, s_rps.GetRPS(OperationType::JOB_RESULT, currentTime)) << std::endl;
    file << Utf8PrintfString("Job Cancel   %6d %10d %9d %10d %9d        %f", opStats[OperationType::JOB_CANCEL]->success, opStats[OperationType::JOB_CANCEL]->failure, (int)opStats[OperationType::JOB_CANCEL]->minTime, (int)opStats[OperationType::JOB_CANCEL]->maxTime, (int)opStats[OperationType::JOB_CANCEL]->avgTime, s_rps.GetRPS(OperationType::JOB_CANCEL, currentTime)) << std::endl;

    file << std::endl << std::endl << "op list:" << std::endl;

    for (Utf8String op : opLog)
        file << op << std::endl;

    file << std::endl << std::endl << "error list:" << std::endl;
    
    file << "List:" << std::endl;
    for (Utf8String error : errors[OperationType::LIST_PROJECT])
        file << error << std::endl;

    file << "NavNode:" << std::endl;
    for (Utf8String error : errors[OperationType::ADD_PROJECT])
        file << error << std::endl;

    file << "Stat:" << std::endl;
    for (Utf8String error : errors[OperationType::DELETE_PROJECT])
        file << error << std::endl;

    file << "Details:" << std::endl;
    for (Utf8String error : errors[OperationType::GET_PROJECT])
        file << error << std::endl;

    file << "AzureAddress:" << std::endl;
    for (Utf8String error : errors[OperationType::SAS_URI])
        file << error << std::endl;

    file << "Create:" << std::endl;
    for (Utf8String error : errors[OperationType::LIST_CLUSTERS])
        file << error << std::endl;

    file << "Link:" << std::endl;
    for (Utf8String error : errors[OperationType::CREATE_JOB])
        file << error << std::endl;

    file << "Unlink:" << std::endl;
    for (Utf8String error : errors[OperationType::ADD_JOB])
        file << error << std::endl;

    file << "Delete:" << std::endl;
    for (Utf8String error : errors[OperationType::DELETE_JOB])
        file << error << std::endl;

    file << "Download:" << std::endl;
    for (Utf8String error : errors[OperationType::GET_JOBS])
        file << error << std::endl;

    file << "Download:" << std::endl;
    for (Utf8String error : errors[OperationType::GET_JOB])
        file << error << std::endl;

    file << "Download:" << std::endl;
    for (Utf8String error : errors[OperationType::JOB_RESULT])
        file << error << std::endl;

    file << "Download:" << std::endl;
    for (Utf8String error : errors[OperationType::JOB_CANCEL])
        file << error << std::endl;

    file.close();
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
User::User():
    m_currentOperation(OperationType::LIST_PROJECT), m_handshake(AzureHandshake())
    {}

User::User(int id) :
    m_currentOperation(OperationType::LIST_PROJECT), m_handshake(AzureHandshake()),
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
    else if (m_currentOperation == OperationType::ADD_JOB)
        {
        if (m_jobId.empty())
            m_currentOperation = OperationType::CREATE_JOB;
        else
            curl = CreateJob();
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
    else if (m_currentOperation == OperationType::JOB_RESULT)
        {
        if (m_jobId.empty())
            m_currentOperation = OperationType::CREATE_JOB;
        else
            curl = GetJobResult();
        }
    else if (m_currentOperation == OperationType::JOB_CANCEL)
        {
        if (m_jobId.empty())
            m_currentOperation = OperationType::CREATE_JOB;
        else
            curl = CancelJob();
        }
    else if (m_currentOperation == OperationType::LIST_CLUSTERS)
        {
        if (m_id.empty())
            m_currentOperation = OperationType::CREATE_JOB;
        else
            curl = ListClusters();
        }


    if(m_currentOperation == OperationType::LIST_PROJECT)
        {
        curl = ListProject();
        }
    else if (m_currentOperation == OperationType::ADD_PROJECT)
        {
        curl = AddProject();
        }
    else if (m_currentOperation == OperationType::CREATE_JOB)
        {
        curl = CreateJob();
        }
    else if (m_currentOperation == OperationType::GET_JOBS)
        {
        curl = GetJobs();
        }

    m_start = std::time(nullptr);
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

    m_currentOperation = OperationType::DELETE_PROJECT;
    CURL* curl = DeleteProject();
    
    m_start = std::time(nullptr);
    if (curl != nullptr)
        owner->SetupCurl(curl, this);
    }

CURL* User::ListProject()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append("/api/v1/projects");
    m_correspondance.req.type = GET;

    return PrepareRequest();
    }

CURL* User::AddProject()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append("/api/v1/projects");
    m_correspondance.req.type = POST;
    m_correspondance.req.payload = "{\"region\": \"eus\", \"name\":\"something\"}";

    m_id = Utf8PrintfString("%d", m_userId);

    return PrepareRequest();
    }

CURL* User::DeleteProject()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("/api/v1/projects/%s", m_id));
    m_correspondance.req.type = DEL;

    return PrepareRequest();
    }

CURL* User::GetProject()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("/api/v1/projects/%s", m_id));
    m_correspondance.req.type = GET;

    return PrepareRequest();
    }

CURL* User::SASUri()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("/api/v1/projects/%s/sas-uri", m_id));
    m_correspondance.req.type = GET;

    return PrepareRequest();
    }

CURL* User::ListClusters()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("/api/v1/projects/%s/clusters", m_id));
    m_correspondance.req.type = GET;

    return PrepareRequest();
    }

CURL* User::CreateJob()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("/api/v1/projects/%s/jobs", m_id));
    m_correspondance.req.type = POST;
    Utf8String body =   "{"
                            "\"id\": \"%s\","
                            "\"userId\" : \"string\","
                            "\"creationTime\" : \"2017-04-26T18:25:35.450Z\","
                            "\"projectId\" : \"string\","
                            "\"connectProjectId\" : \"string\","
                            "\"federatedRepositoryId\" : \"string\","
                            "\"clusterId\" : \"string\","
                            "\"projectName\" : \"string\","
                            "\"jobType\" : 0,"
                            "\"priority\" : 0,"
                            "\"inputs\" : {"
                                "\"storage\": {"
                                    "\"storageType\": 0,"
                                    "\"path\" : \"string\""
                                "}"
                            "},"
                            "\"submissionDetails\": {"
                                "\"hostname\": \"string\","
                                "\"email\" : \"string\","
                                "\"additionalEmails\" : ["
                                    "\"string\""
                                "],"
                                "\"organizationId\" : \"string\","
                                "\"time\" : \"2017-04-26T18:25:35.450Z\""
                            "},"
                            "\"status\": 0,"
                            "\"settings\" : {"
                                "\"publicWebGL\": true,"
                                "\"meshQuality\" : 0,"
                                "\"outputs\" : ["
                                    "{"
                                        "\"format\": \"string\","
                                        "\"storages\" : ["
                                            "{"
                                                "\"storageType\": 0,"
                                                "\"path\" : \"string\""
                                            "}"
                                        "]"
                                    "}"
                                "]"
                            "},"
                            "\"result\": {"
                                "\"resultUrl\": \"string\","
                                "\"errorMessage\" : \"string\","
                                "\"completionTimeMs\" : 0,"
                                "\"returnCode\" : 0"
                            "}"
                        "}";

    m_correspondance.req.payload = Utf8PrintfString(body.c_str(), m_id);
    m_jobId = m_id;

    return PrepareRequest();
    }

CURL* User::AddJobForId()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("/api/v1/projects/%s/jobs/%s", m_id, m_jobId));
    m_correspondance.req.type = POST;

    return PrepareRequest();
    }

CURL* User::DeleteJob()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("/api/v1/jobs/%s", m_jobId));
    m_correspondance.req.type = DEL;

    return PrepareRequest();
    }   

CURL* User::GetJobs()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("/api/v1/jobs/%s", m_id));
    m_correspondance.req.type = GET;

    return PrepareRequest();
    }

CURL* User::GetJobById()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("/api/v1/ws/jobs/%s", m_jobId));
    m_correspondance.req.type = GET;

    return PrepareRequest();
    }

CURL* User::GetJobResult()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append("/api/v1/jobs-result");
    m_correspondance.req.type = POST;

    return PrepareRequest();
    }

CURL* User::CancelJob()
    {
    m_correspondance.Clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("/api/v1/jobs/%s/cancel", m_jobId));
    m_correspondance.req.type = POST;

    return PrepareRequest();
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* automatically call the proper validation function for the last executed operation
//+---------------+---------------+---------------+---------------+---------------+------*/
void User::ValidatePrevious(int activeUsers)
    {
    s_stats.InsertStats(this, (m_correspondance.response.curlCode == CURLE_OK), activeUsers);
    /*if(m_correspondance.response.curlCode != CURLE_OK)
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
        case OperationType::CHANGE:
            return ValidateChange(activeUsers);
        case OperationType::LINK:
            return ValidateLink(activeUsers);
        case OperationType::UNLINK:
            return ValidateUnlink(activeUsers);
        case OperationType::REMOVE:
            return ValidateDelete(activeUsers);
        case OperationType::LIST:
            return ValidateList(activeUsers);
        case OperationType::CREATE:
            return ValidateCreate(activeUsers);
        case OperationType::STAT:
            return ValidateStat(activeUsers);
        case OperationType::DOWNLOAD:
            return ValidateDownload(activeUsers);
        }*/
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
User::~User()
    {}

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
        User* user = s_innactiveUsers.back();
        s_innactiveUsers.pop_back();
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
    if (m_correspondance.req.type == requestType::POST)
        {
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

    bvector<Utf8String> wsgHeaders = m_correspondance.req.headers;
    for (Utf8String header : wsgHeaders)
        headers = curl_slist_append(headers, header.c_str());

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