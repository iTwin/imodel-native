/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
#include <RealityPlatformTools/RealityConversionTools.h>
#include <CCApi/CCPublic.h>

#include "3CSLoadTest.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

std::mutex guidMutex;

static Utf8String s_server("https://qa-connect-contextcapture.bentley.com/");
static bvector<Utf8String> s_guidparts = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f"};

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
//* request per second class
//* for each operation type, keep a cout of how many requests were sent at any given second
//+---------------+---------------+---------------+---------------+---------------+------*/
ThreeCSRPS::ThreeCSRPS() : RPS()
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
//* for each operation type, tracks success and failures, as well as response time
//+---------------+---------------+---------------+---------------+---------------+------*/
ThreeCSStats::ThreeCSStats(ThreeCSRPS* rps) : Stats(rps)
    {   
    opStats = bmap<int, Stat*>();
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

    errors = bmap<int, bvector<Utf8String>>();
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
void ThreeCSStats::PrintStatsBody(int64_t currentTime, int& totalSuccess, int& totalFailure)
    {
    system("cls");

    std::cout << "Type        Success    Failure   minTime   maxTime   avgTime  requests/second" << std::endl;
                                                                       
    std::cout << Utf8PrintfString("List project %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_PROJECT]->success, opStats[OperationType::LIST_PROJECT]->failure, (int)opStats[OperationType::LIST_PROJECT]->minTime, (int)opStats[OperationType::LIST_PROJECT]->maxTime, (int)opStats[OperationType::LIST_PROJECT]->avgTime, m_rps->GetRPS(OperationType::LIST_PROJECT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Add project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::ADD_PROJECT]->success, opStats[OperationType::ADD_PROJECT]->failure, (int)opStats[OperationType::ADD_PROJECT]->minTime, (int)opStats[OperationType::ADD_PROJECT]->maxTime, (int)opStats[OperationType::ADD_PROJECT]->avgTime, m_rps->GetRPS(OperationType::ADD_PROJECT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Del project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_PROJECT]->success, opStats[OperationType::DELETE_PROJECT]->failure, (int)opStats[OperationType::DELETE_PROJECT]->minTime, (int)opStats[OperationType::DELETE_PROJECT]->maxTime, (int)opStats[OperationType::DELETE_PROJECT]->avgTime, m_rps->GetRPS(OperationType::DELETE_PROJECT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Get project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_PROJECT]->success, opStats[OperationType::GET_PROJECT]->failure, (int)opStats[OperationType::GET_PROJECT]->minTime, (int)opStats[OperationType::GET_PROJECT]->maxTime, (int)opStats[OperationType::GET_PROJECT]->avgTime, m_rps->GetRPS(OperationType::GET_PROJECT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("SAS URI      %6d %10d %9d %10d %9d        %f", opStats[OperationType::SAS_URI]->success, opStats[OperationType::SAS_URI]->failure, (int)opStats[OperationType::SAS_URI]->minTime, (int)opStats[OperationType::SAS_URI]->maxTime, (int)opStats[OperationType::SAS_URI]->avgTime, m_rps->GetRPS(OperationType::SAS_URI, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("List Cluster %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_CLUSTERS]->success, opStats[OperationType::LIST_CLUSTERS]->failure, (int)opStats[OperationType::LIST_CLUSTERS]->minTime, (int)opStats[OperationType::LIST_CLUSTERS]->maxTime, (int)opStats[OperationType::LIST_CLUSTERS]->avgTime, m_rps->GetRPS(OperationType::LIST_CLUSTERS, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Create Job   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CREATE_JOB]->success, opStats[OperationType::CREATE_JOB]->failure, (int)opStats[OperationType::CREATE_JOB]->minTime, (int)opStats[OperationType::CREATE_JOB]->maxTime, (int)opStats[OperationType::CREATE_JOB]->avgTime, m_rps->GetRPS(OperationType::CREATE_JOB, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Submit Job   %6d %10d %9d %10d %9d        %f", opStats[OperationType::SUBMIT_JOB]->success, opStats[OperationType::SUBMIT_JOB]->failure, (int)opStats[OperationType::SUBMIT_JOB]->minTime, (int)opStats[OperationType::SUBMIT_JOB]->maxTime, (int)opStats[OperationType::SUBMIT_JOB]->avgTime, m_rps->GetRPS(OperationType::SUBMIT_JOB, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Del Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_JOB]->success, opStats[OperationType::DELETE_JOB]->failure, (int)opStats[OperationType::DELETE_JOB]->minTime, (int)opStats[OperationType::DELETE_JOB]->maxTime, (int)opStats[OperationType::DELETE_JOB]->avgTime, m_rps->GetRPS(OperationType::DELETE_JOB, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Get Jobs     %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_JOBS]->success, opStats[OperationType::GET_JOBS]->failure, (int)opStats[OperationType::GET_JOBS]->minTime, (int)opStats[OperationType::GET_JOBS]->maxTime, (int)opStats[OperationType::GET_JOBS]->avgTime, m_rps->GetRPS(OperationType::GET_JOBS, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Get Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_JOB]->success, opStats[OperationType::GET_JOB]->failure, (int)opStats[OperationType::GET_JOB]->minTime, (int)opStats[OperationType::GET_JOB]->maxTime, (int)opStats[OperationType::GET_JOB]->avgTime, m_rps->GetRPS(OperationType::GET_JOB, currentTime)) << std::endl;
    //std::cout << Utf8PrintfString("Job Result   %6d %10d %9d %10d %9d        %f", opStats[OperationType::JOB_RESULT]->success, opStats[OperationType::JOB_RESULT]->failure, (int)opStats[OperationType::JOB_RESULT]->minTime, (int)opStats[OperationType::JOB_RESULT]->maxTime, (int)opStats[OperationType::JOB_RESULT]->avgTime, m_rps->GetRPS(OperationType::JOB_RESULT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Job Cancel   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CANCEL_JOB]->success, opStats[OperationType::CANCEL_JOB]->failure, (int)opStats[OperationType::CANCEL_JOB]->minTime, (int)opStats[OperationType::CANCEL_JOB]->maxTime, (int)opStats[OperationType::CANCEL_JOB]->avgTime, m_rps->GetRPS(OperationType::CANCEL_JOB, currentTime)) << std::endl;

    std::cout << "active users: " << m_activeUsers << std::endl << std::endl;

    totalSuccess = opStats[OperationType::LIST_PROJECT]->success + opStats[OperationType::ADD_PROJECT]->success +
        opStats[OperationType::DELETE_PROJECT]->success + opStats[OperationType::GET_PROJECT]->success +
        opStats[OperationType::SAS_URI]->success + opStats[OperationType::LIST_CLUSTERS]->success + 
        opStats[OperationType::CREATE_JOB]->success + opStats[OperationType::SUBMIT_JOB]->success + 
        opStats[OperationType::DELETE_JOB]->success + opStats[OperationType::GET_JOBS]->success +
        opStats[OperationType::GET_JOB]->success + opStats[OperationType::CANCEL_JOB]->success;

    totalFailure = opStats[OperationType::LIST_PROJECT]->failure + opStats[OperationType::ADD_PROJECT]->failure +
        opStats[OperationType::DELETE_PROJECT]->failure + opStats[OperationType::GET_PROJECT]->failure +
        opStats[OperationType::SAS_URI]->failure + opStats[OperationType::LIST_CLUSTERS]->failure + 
        opStats[OperationType::CREATE_JOB]->failure + opStats[OperationType::SUBMIT_JOB]->failure +
        opStats[OperationType::DELETE_JOB]->failure + opStats[OperationType::GET_JOBS]->failure +
        opStats[OperationType::GET_JOB]->failure + opStats[OperationType::CANCEL_JOB]->failure;

    std::cout << Utf8PrintfString("Total                 %6d %10d", totalSuccess, totalFailure) << std::endl;
    std::cout << Utf8PrintfString("Total Requests        %6d", totalSuccess + totalFailure) << std::endl;

    std::cout << std::endl;

    std::cout << "Press any key to quit testing" << std::endl;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeCSStats::WriteToFileBody(int userCount, Utf8String path, std::ofstream& file)
    {
    file << Utf8PrintfString("List project %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_PROJECT]->success, opStats[OperationType::LIST_PROJECT]->failure, (int)opStats[OperationType::LIST_PROJECT]->minTime, (int)opStats[OperationType::LIST_PROJECT]->maxTime, (int)opStats[OperationType::LIST_PROJECT]->avgTime) << std::endl;
    file << Utf8PrintfString("Add project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::ADD_PROJECT]->success, opStats[OperationType::ADD_PROJECT]->failure, (int)opStats[OperationType::ADD_PROJECT]->minTime, (int)opStats[OperationType::ADD_PROJECT]->maxTime, (int)opStats[OperationType::ADD_PROJECT]->avgTime) << std::endl;
    file << Utf8PrintfString("Del project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_PROJECT]->success, opStats[OperationType::DELETE_PROJECT]->failure, (int)opStats[OperationType::DELETE_PROJECT]->minTime, (int)opStats[OperationType::DELETE_PROJECT]->maxTime, (int)opStats[OperationType::DELETE_PROJECT]->avgTime) << std::endl;
    file << Utf8PrintfString("Get project  %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_PROJECT]->success, opStats[OperationType::GET_PROJECT]->failure, (int)opStats[OperationType::GET_PROJECT]->minTime, (int)opStats[OperationType::GET_PROJECT]->maxTime, (int)opStats[OperationType::GET_PROJECT]->avgTime) << std::endl;
    file << Utf8PrintfString("SAS URI      %6d %10d %9d %10d %9d        %f", opStats[OperationType::SAS_URI]->success, opStats[OperationType::SAS_URI]->failure, (int)opStats[OperationType::SAS_URI]->minTime, (int)opStats[OperationType::SAS_URI]->maxTime, (int)opStats[OperationType::SAS_URI]->avgTime) << std::endl;
    file << Utf8PrintfString("List Cluster %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_CLUSTERS]->success, opStats[OperationType::LIST_CLUSTERS]->failure, (int)opStats[OperationType::LIST_CLUSTERS]->minTime, (int)opStats[OperationType::LIST_CLUSTERS]->maxTime, (int)opStats[OperationType::LIST_CLUSTERS]->avgTime) << std::endl;
    file << Utf8PrintfString("Create Job   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CREATE_JOB]->success, opStats[OperationType::CREATE_JOB]->failure, (int)opStats[OperationType::CREATE_JOB]->minTime, (int)opStats[OperationType::CREATE_JOB]->maxTime, (int)opStats[OperationType::CREATE_JOB]->avgTime) << std::endl;
    file << Utf8PrintfString("Submit Job   %6d %10d %9d %10d %9d        %f", opStats[OperationType::SUBMIT_JOB]->success, opStats[OperationType::SUBMIT_JOB]->failure, (int)opStats[OperationType::SUBMIT_JOB]->minTime, (int)opStats[OperationType::SUBMIT_JOB]->maxTime, (int)opStats[OperationType::SUBMIT_JOB]->avgTime) << std::endl;
    file << Utf8PrintfString("Del Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_JOB]->success, opStats[OperationType::DELETE_JOB]->failure, (int)opStats[OperationType::DELETE_JOB]->minTime, (int)opStats[OperationType::DELETE_JOB]->maxTime, (int)opStats[OperationType::DELETE_JOB]->avgTime) << std::endl;
    file << Utf8PrintfString("Get Jobs     %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_JOBS]->success, opStats[OperationType::GET_JOBS]->failure, (int)opStats[OperationType::GET_JOBS]->minTime, (int)opStats[OperationType::GET_JOBS]->maxTime, (int)opStats[OperationType::GET_JOBS]->avgTime) << std::endl;
    file << Utf8PrintfString("Get Job      %6d %10d %9d %10d %9d        %f", opStats[OperationType::GET_JOB]->success, opStats[OperationType::GET_JOB]->failure, (int)opStats[OperationType::GET_JOB]->minTime, (int)opStats[OperationType::GET_JOB]->maxTime, (int)opStats[OperationType::GET_JOB]->avgTime) << std::endl;
    //file << Utf8PrintfString("Job Result   %6d %10d %9d %10d %9d        %f", opStats[OperationType::JOB_RESULT]->success, opStats[OperationType::JOB_RESULT]->failure, (int)opStats[OperationType::JOB_RESULT]->minTime, (int)opStats[OperationType::JOB_RESULT]->maxTime, (int)opStats[OperationType::JOB_RESULT]->avgTime) << std::endl;
    file << Utf8PrintfString("Job Cancel   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CANCEL_JOB]->success, opStats[OperationType::CANCEL_JOB]->failure, (int)opStats[OperationType::CANCEL_JOB]->minTime, (int)opStats[OperationType::CANCEL_JOB]->maxTime, (int)opStats[OperationType::CANCEL_JOB]->avgTime) << std::endl;

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
ThreeCSUser::ThreeCSUser(int id, Utf8String token, Utf8String rdGuid, Stats* stats) : User(id, stats),
    m_token(token), m_RDGuid(rdGuid)
    {}

Utf8String rc()
    {
    return s_guidparts[rand() % s_guidparts.size()];
    }

Utf8String generateGuid()
    {
    return Utf8PrintfString("%s%s%s%s%s%s%s%s-%s%s%s%s-%s%s%s%s-%s%s%s%s-%s%s%s%s%s%s%s%s%s%s%s%s",
        rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), 
        rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str(), rc().c_str());
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* prepares next request, based on most recent action
//* can be called blindly from outside
//+---------------+---------------+---------------+---------------+---------------+------*/
bool ThreeCSUser::DoNextBody(UserManager* owner)
    {
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
        else
            {
            curl = CreateJob(generateGuid());
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
        {
        owner->SetupCurl(curl, this);
        return false;
        }
    else
        return true;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* prepares next request, based on most recent action
//* can be called blindly from outside
//+---------------+---------------+---------------+---------------+---------------+------*/
bool ThreeCSUser::WrapUp(UserManager* owner)
    {
    CURL* curl = nullptr;
    bool somethingToDo = false;
    
    if (!m_jobId.empty())
        {
        m_currentOperation = OperationType::DELETE_JOB;
        curl = DeleteJob();
        somethingToDo = true;
        }
    else if (!m_id.empty())
        {
        m_currentOperation = OperationType::DELETE_PROJECT;
        curl = DeleteProject();
        somethingToDo = true;
        }

    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(m_start);
    if (curl != nullptr)
        owner->SetupCurl(curl, this);
    else if (somethingToDo)
        return true;
    else
        m_wrappedUp = true;
        
    return false;
    }

CURL* ThreeCSUser::ListProject()
    {
    m_correspondance.response.clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append("api/v1/projects");

    m_correspondance.id = m_stats->LogRequest("List projects");

    return PrepareRequest();
    }

CURL* ThreeCSUser::AddProject()
    {
    m_correspondance.response.clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append("api/v1/projects");
    m_correspondance.req.payload = Utf8PrintfString("{\"region\": \"eus\", \"connectProjectId\":\"%s\", \"name\":\"Load Test (ERASE)\"}", m_RDGuid.c_str());

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("Add project %s", m_RDGuid.c_str()));
    
    return PrepareRequest();
    }

CURL* ThreeCSUser::DeleteProject()
    {
    m_correspondance.response.clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/projects/%s", m_id.c_str()));

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("Delete project %s", m_id.c_str()));

    m_id.clear();

    return PrepareRequest();
    }

CURL* ThreeCSUser::GetProject()
    {
    m_correspondance.response.clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/projects/%s", m_id.c_str()));

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("Get project %s", m_id.c_str()));

    return PrepareRequest();
    }

CURL* ThreeCSUser::SASUri()
    {
    m_correspondance.response.clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/projects/%s/sas-uri", m_id.c_str()));

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("SAS Uri for %s", m_id.c_str()));

    return PrepareRequest();
    }

CURL* ThreeCSUser::ListClusters()
    {
    m_correspondance.response.clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/projects/%s/clusters", m_id.c_str()));

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("List Clusters for %s", m_id.c_str()));

    return PrepareRequest();
    }

CURL* ThreeCSUser::CreateJob(Utf8String outputGuid)
    {
    m_correspondance.response.clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append("api/v1/jobs");
    Utf8String body =   "{"
                            "\"projectName\" : \"load test job\","
                            "\"jobType\" : \"Photos2Mesh\","
                            "\"projectId\": \"%s\","
                            "\"inputs\" : {"
                                "\"storage\":{"
                                    "\"storageType\":\"Rds\","
                                    "\"path\" : \"%s\""
                                "}"
                            "},"
                            "\"submissionDetails\":{"
                                "\"hostname\":\"EMEA84158\","
                                "\"email\" : \"Emmanuel.Pot@bentley.com\","
                                "\"organizationId\" : \"e82a584b-9fae-409f-9581-fd154f7b9ef9\""
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
    
    m_correspondance.req.payload = Utf8PrintfString(body.c_str(), m_id.c_str(), m_RDGuid.c_str(), outputGuid.c_str());

    m_submitted = false;

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("Create Job %s for %s", m_id.c_str(), m_RDGuid.c_str()));

    return PrepareRequest();
    }

CURL* ThreeCSUser::SubmitJob()
    {
    m_correspondance.response.clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/jobs/%s/submit", m_jobId.c_str()));

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("Submit job %s", m_jobId.c_str()));

    return PrepareRequest();
    }

CURL* ThreeCSUser::DeleteJob()
    {
    m_correspondance.response.clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/jobs/%s", m_jobId.c_str()));

    m_submitted = false;

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("Delete job %s", m_jobId.c_str()));

    m_jobId.clear();

    return PrepareRequest();
    }   

CURL* ThreeCSUser::GetJobs()
    {
    m_correspondance.response.clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append("api/v1/jobs");

    m_correspondance.id = m_stats->LogRequest("Get Jobs");

    return PrepareRequest();
    }

CURL* ThreeCSUser::GetJobById()
    {
    m_correspondance.response.clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/jobs/%s", m_jobId.c_str()));

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("Get Job %s", m_jobId.c_str()));

    return PrepareRequest();
    }

/*CURL* ThreeCSUser::GetJobResult()
    {
    m_correspondance.response.clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append("api/v1/job-result");

    return PrepareRequest();
    }*/

CURL* ThreeCSUser::CancelJob()
    {
    m_correspondance.response.clear();
    m_correspondance.req.url = s_server;
    m_correspondance.req.url.append(Utf8PrintfString("api/v1/jobs/%s/cancel", m_jobId.c_str()));

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("Cancel job %s", m_jobId.c_str()));

    return PrepareRequest();
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* automatically call the proper validation function for the last executed operation
//+---------------+---------------+---------------+---------------+---------------+------*/
void ThreeCSUser::ValidatePrevious(int activeUsers)
    {
    switch (m_currentOperation)
        {
        case OperationType::ADD_PROJECT:
            return ValidateAddProject(activeUsers);
        case OperationType::CREATE_JOB:
            return ValidateCreateJob(activeUsers);
        case OperationType::SUBMIT_JOB:
            return ValidateSubmitJob(activeUsers);
        case OperationType::CANCEL_JOB:
            return ValidateCancelJob(activeUsers);
        default:
            {
            if(m_correspondance.response.toolCode != CURLE_OK || m_correspondance.response.responseCode > 399)
                {
                m_stats->InsertStats(this, false, activeUsers);
                return;
                }
            else
                m_stats->InsertStats(this, (m_correspondance.response.toolCode == CURLE_OK), activeUsers);
            }
        }
    }

void ThreeCSUser::ValidateAddProject(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    bool success = false;

    if (m_correspondance.response.ValidateJSONResponse(instances, "id") == RequestStatus::OK)
        {
        success = true;
        m_id = instances["id"].asString();
        }
    
    m_stats->InsertStats(this, success, activeUsers);
    }

void ThreeCSUser::ValidateCreateJob(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    bool success = false;

    if (m_correspondance.response.ValidateJSONResponse(instances, "id") == RequestStatus::OK)
        {
        success = true;
        m_jobId = instances["id"].asString();
        m_submitted = false;
        }

    m_stats->InsertStats(this, success, activeUsers);
    }

void ThreeCSUser::ValidateSubmitJob(int activeUsers)
{
    Json::Value instances(Json::objectValue);

    bool success = false;
    if(m_correspondance.response.body.ContainsI("InstanceId already used"))
        return;

    if (m_correspondance.response.toolCode == CURLE_OK && m_correspondance.response.responseCode < 399)
        {
        success = true;
        m_submitted = true;
        }

    m_stats->InsertStats(this, success, activeUsers);
}

void ThreeCSUser::ValidateCancelJob(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    bool success = false;

    if (m_correspondance.response.responseCode < 399 || m_correspondance.response.body.ContainsI("Job does not seem to be pending nor running") || m_correspondance.response.body.ContainsI("\"code\":204"))
        {
        success = true;
        m_submitted = false;
        }

    m_stats->InsertStats(this, success, activeUsers);
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
int main(int argc, char* argv[])
    {
    SetConsoleTitle("3CS Load Test");

    LoadTester tester = LoadTester();
    ThreeCSRPS rps = ThreeCSRPS();
    ThreeCSStats stats = ThreeCSStats(&rps);
    Utf8String rdGuid; //4593372e-c1df-41c0-aa91-6f9816b9519b

    LoadTester::SetupStaticClasses(&stats, &rps);
    if (tester.Main(argc, argv))
        return 0;
        
    char* substringPosition;
    for (int i = 0; i < argc; ++i)
        {
        if (strstr(argv[i], "-g:") || strstr(argv[i], "--guid:"))
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            rdGuid = Utf8String(substringPosition);
            }
        }

    if (rdGuid.empty())
        {
        std::cout << "  -g, --guid  A valid RealityData Guid is required to submit jobs" << std::endl;
        return -1;
        }

    CCAPIHANDLE api = CCApi_InitializeApi(COM_THREADING_Multi);
    CallStatus status = APIERR_SUCCESS;

    bool installed;
    status = CCApi_IsInstalled(api, &installed);
    if (!installed)
        {
        std::cout << "Connection client does not seem to be installed" << std::endl;
        CCApi_FreeApi(api);
        return -1;
        }
    bool running = false;
    status = CCApi_IsRunning(api, &running);
    if (status != APIERR_SUCCESS || !running)
        {
        std::cout << "Connection client does not seem to be running" << std::endl;
        CCApi_FreeApi(api);
        return -1;
        }
    bool loggedIn = false;
    status = CCApi_IsLoggedIn(api, &loggedIn);
    if (status != APIERR_SUCCESS || !loggedIn)
        {
        std::cout << "Connection client does not seem to be logged in" << std::endl;
        CCApi_FreeApi(api);
        return -1;
        }
    bool acceptedEula = false;
    status = CCApi_HasUserAcceptedEULA(api, &acceptedEula);
    if (status != APIERR_SUCCESS || !acceptedEula)
        {
        std::cout << "Connection client user does not seem to have accepted EULA" << std::endl;
        CCApi_FreeApi(api);
        return -1;
        }
    bool sessionActive = false;
    status = CCApi_IsUserSessionActive(api, &sessionActive);
    if (status != APIERR_SUCCESS || !sessionActive)
        {
        std::cout << "Connection client does not seem to have an active session" << std::endl;
        CCApi_FreeApi(api);
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

    CCApi_FreeApi(api);

    std::queue<User*> inactiveUsers;

    if (!tester.trickle)
        {
        for (int i = 0; i < tester.userManager.m_userCount; i++)
            {
            tester.userManager.users.push_back(new ThreeCSUser(i, token, rdGuid, &stats));
            }
        }
    else
        {
        tester.userManager.users.push_back(new ThreeCSUser(0, token, rdGuid, &stats)); //start with one
        for (int i = 1; i < tester.userManager.m_userCount; i++)
            {
            inactiveUsers.push(new ThreeCSUser(i, token, rdGuid, &stats)); //feed the rest to the Dispatcher
            }
        }

    if (tester.m_serverType == RealityPlatform::CONNECTServerType::PROD)
        s_server = "https://dev-contextcapture-eus.cloudapp.net/";
    else if (tester.m_serverType == RealityPlatform::CONNECTServerType::QA)
        s_server = "https://qa-connect-contextcapture.bentley.com/";
    else // (tester.m_serverType == RealityPlatform::CONNECTServerType::DEV)
        s_server = "https://dev-contextcapture-eus.cloudapp.net/";

    tester.SetupInactiveUsers(inactiveUsers);

    //RealityDataService::SetProjectId(projectId); // 0ae1fcca-ead6-4e94-b83f-38d25db1b16e

    tester.Main2(0);
    stats.WriteToFile(tester.userManager.m_userCount, tester.path);
    }

CURL* ThreeCSUser::PrepareRequest()
    {
    CURL* curl = curl_easy_init();
    if (nullptr == curl)
        {
        m_correspondance.response.toolCode = CURLcode::CURLE_FAILED_INIT;
        return curl;
        }

    //Adjusting headers for the POST method
    struct curl_slist *headers = NULL;

    headers = curl_slist_append(headers, m_token.c_str());

    if (m_currentOperation >= OperationType::DELETE_PROJECT )
        {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, m_correspondance.req.payload.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, m_correspondance.req.payload.length());
        }
    else if (m_currentOperation >= OperationType::ADD_PROJECT)
        {
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, m_correspondance.req.payload.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, m_correspondance.req.payload.length());
        }

    curl_easy_setopt(curl, CURLOPT_URL, m_correspondance.req.url.c_str());

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE);

    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &(m_correspondance.response.header));

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(m_correspondance.response.body));

    return curl;
    }