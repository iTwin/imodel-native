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

#include "RDSLoadTest.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* request per second class
//* for each operation type, keep a cout of how many requests were sent at any given second
//+---------------+---------------+---------------+---------------+---------------+------*/
RDSRPS::RDSRPS(): RPS()
    {
    requestLog.Insert(OperationType::LIST_REALITYDATA, bmap<int64_t, int>());
    requestLog.Insert(OperationType::LIST_RELATIONSHIP, bmap<int64_t, int>());
    requestLog.Insert(OperationType::ENTERPRISE_STAT, bmap<int64_t, int>());
    requestLog.Insert(OperationType::DETAILS, bmap<int64_t, int>());
    requestLog.Insert(OperationType::AZURE_ADDRESS, bmap<int64_t, int>());
    requestLog.Insert(OperationType::CREATE_REALITYDATA, bmap<int64_t, int>());
    requestLog.Insert(OperationType::MODIFY_REALITYDATA, bmap<int64_t, int>());
    requestLog.Insert(OperationType::CREATE_RELATIONSHIP, bmap<int64_t, int>());
    requestLog.Insert(OperationType::DELETE_RELATIONSHIP, bmap<int64_t, int>());
    requestLog.Insert(OperationType::DELETE_REALITYDATA, bmap<int64_t, int>());
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* for each operation type, tracks success and failures, as well as response time
//+---------------+---------------+---------------+---------------+---------------+------*/
RDSStats::RDSStats(RDSRPS* rps) : Stats(rps)
    {   
    opStats = bmap<int, Stat*>();
    opStats.Insert(OperationType::LIST_REALITYDATA, new Stat());
    opStats.Insert(OperationType::LIST_RELATIONSHIP, new Stat());
    opStats.Insert(OperationType::ENTERPRISE_STAT, new Stat());
    opStats.Insert(OperationType::DETAILS, new Stat());
    opStats.Insert(OperationType::AZURE_ADDRESS, new Stat());
    opStats.Insert(OperationType::CREATE_REALITYDATA, new Stat());
    opStats.Insert(OperationType::MODIFY_REALITYDATA, new Stat());
    opStats.Insert(OperationType::CREATE_RELATIONSHIP, new Stat());
    opStats.Insert(OperationType::DELETE_RELATIONSHIP, new Stat());
    opStats.Insert(OperationType::DELETE_REALITYDATA, new Stat());

    errors = bmap<int, bvector<Utf8String>>();
    errors.Insert(OperationType::LIST_REALITYDATA, bvector<Utf8String>());
    errors.Insert(OperationType::LIST_RELATIONSHIP, bvector<Utf8String>());
    errors.Insert(OperationType::ENTERPRISE_STAT, bvector<Utf8String>());
    errors.Insert(OperationType::DETAILS, bvector<Utf8String>());
    errors.Insert(OperationType::AZURE_ADDRESS, bvector<Utf8String>());
    errors.Insert(OperationType::CREATE_REALITYDATA, bvector<Utf8String>());
    errors.Insert(OperationType::MODIFY_REALITYDATA, bvector<Utf8String>());
    errors.Insert(OperationType::CREATE_RELATIONSHIP, bvector<Utf8String>());
    errors.Insert(OperationType::DELETE_RELATIONSHIP, bvector<Utf8String>());
    errors.Insert(OperationType::DELETE_REALITYDATA, bvector<Utf8String>());
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void RDSStats::PrintStatsBody(int64_t currentTime, int& totalSuccess, int& totalFailure)
    {
    system("cls");

    std::cout << "Type                    Success    Failure   minTime   maxTime   avgTime  requests/second" << std::endl;
    
    std::cout << Utf8PrintfString("Create Reality Data   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CREATE_REALITYDATA]->success, opStats[OperationType::CREATE_REALITYDATA]->failure, (int)opStats[OperationType::CREATE_REALITYDATA]->minTime, (int)opStats[OperationType::CREATE_REALITYDATA]->maxTime, (int)opStats[OperationType::CREATE_REALITYDATA]->avgTime, m_rps->GetRPS(OperationType::CREATE_REALITYDATA, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Create Relationship   %6d %10d %9d %10d %9d        %f", opStats[OperationType::CREATE_RELATIONSHIP]->success, opStats[OperationType::CREATE_RELATIONSHIP]->failure, (int)opStats[OperationType::CREATE_RELATIONSHIP]->minTime, (int)opStats[OperationType::CREATE_RELATIONSHIP]->maxTime, (int)opStats[OperationType::CREATE_RELATIONSHIP]->avgTime, m_rps->GetRPS(OperationType::CREATE_RELATIONSHIP, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Details               %6d %10d %9d %10d %9d        %f", opStats[OperationType::DETAILS]->success, opStats[OperationType::DETAILS]->failure, (int)opStats[OperationType::DETAILS]->minTime, (int)opStats[OperationType::DETAILS]->maxTime, (int)opStats[OperationType::DETAILS]->avgTime, m_rps->GetRPS(OperationType::DETAILS, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("List Reality Data     %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_REALITYDATA]->success, opStats[OperationType::LIST_REALITYDATA]->failure, (int)opStats[OperationType::LIST_REALITYDATA]->minTime, (int)opStats[OperationType::LIST_REALITYDATA]->maxTime, (int)opStats[OperationType::LIST_REALITYDATA]->avgTime, m_rps->GetRPS(OperationType::LIST_REALITYDATA, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("List Relationship     %6d %10d %9d %10d %9d        %f", opStats[OperationType::LIST_RELATIONSHIP]->success, opStats[OperationType::LIST_RELATIONSHIP]->failure, (int)opStats[OperationType::LIST_RELATIONSHIP]->minTime, (int)opStats[OperationType::LIST_RELATIONSHIP]->maxTime, (int)opStats[OperationType::LIST_RELATIONSHIP]->avgTime, m_rps->GetRPS(OperationType::LIST_RELATIONSHIP, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Enterprise Stat       %6d %10d %9d %10d %9d        %f", opStats[OperationType::ENTERPRISE_STAT]->success, opStats[OperationType::ENTERPRISE_STAT]->failure, (int)opStats[OperationType::ENTERPRISE_STAT]->minTime, (int)opStats[OperationType::ENTERPRISE_STAT]->maxTime, (int)opStats[OperationType::ENTERPRISE_STAT]->avgTime, m_rps->GetRPS(OperationType::ENTERPRISE_STAT, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Change Reality Data   %6d %10d %9d %10d %9d        %f", opStats[OperationType::MODIFY_REALITYDATA]->success, opStats[OperationType::MODIFY_REALITYDATA]->failure, (int)opStats[OperationType::MODIFY_REALITYDATA]->minTime, (int)opStats[OperationType::MODIFY_REALITYDATA]->maxTime, (int)opStats[OperationType::MODIFY_REALITYDATA]->avgTime, m_rps->GetRPS(OperationType::MODIFY_REALITYDATA, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Delete Relationship   %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_RELATIONSHIP]->success, opStats[OperationType::DELETE_RELATIONSHIP]->failure, (int)opStats[OperationType::DELETE_RELATIONSHIP]->minTime, (int)opStats[OperationType::DELETE_RELATIONSHIP]->maxTime, (int)opStats[OperationType::DELETE_RELATIONSHIP]->avgTime, m_rps->GetRPS(OperationType::DELETE_RELATIONSHIP, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("Delete Reality Data   %6d %10d %9d %10d %9d        %f", opStats[OperationType::DELETE_REALITYDATA]->success, opStats[OperationType::DELETE_REALITYDATA]->failure, (int)opStats[OperationType::DELETE_REALITYDATA]->minTime, (int)opStats[OperationType::DELETE_REALITYDATA]->maxTime, (int)opStats[OperationType::DELETE_REALITYDATA]->avgTime, m_rps->GetRPS(OperationType::DELETE_REALITYDATA, currentTime)) << std::endl;
    std::cout << Utf8PrintfString("AzureAddress          %6d %10d %9d %10d %9d        %f", opStats[OperationType::AZURE_ADDRESS]->success, opStats[OperationType::AZURE_ADDRESS]->failure, (int)opStats[OperationType::AZURE_ADDRESS]->minTime, (int)opStats[OperationType::AZURE_ADDRESS]->maxTime, (int)opStats[OperationType::AZURE_ADDRESS]->avgTime, m_rps->GetRPS(OperationType::AZURE_ADDRESS, currentTime)) << std::endl;
    std::cout << std::endl;
    totalSuccess = opStats[OperationType::CREATE_REALITYDATA]->success + opStats[OperationType::CREATE_RELATIONSHIP]->success +
                       opStats[OperationType::DETAILS]->success + opStats[OperationType::LIST_REALITYDATA]->success +
                       opStats[OperationType::LIST_RELATIONSHIP]->success +
                       opStats[OperationType::ENTERPRISE_STAT]->success + opStats[OperationType::MODIFY_REALITYDATA]->success +
                       opStats[OperationType::DELETE_RELATIONSHIP]->success + opStats[OperationType::DELETE_REALITYDATA]->success +
                       opStats[OperationType::AZURE_ADDRESS]->success;

    totalFailure = opStats[OperationType::CREATE_REALITYDATA]->failure + opStats[OperationType::CREATE_RELATIONSHIP]->failure +
                       opStats[OperationType::DETAILS]->failure + opStats[OperationType::LIST_REALITYDATA]->failure +
                       opStats[OperationType::LIST_RELATIONSHIP]->failure +
                       opStats[OperationType::ENTERPRISE_STAT]->failure + opStats[OperationType::MODIFY_REALITYDATA]->failure +
                       opStats[OperationType::DELETE_RELATIONSHIP]->failure + opStats[OperationType::DELETE_REALITYDATA]->failure +
                       opStats[OperationType::AZURE_ADDRESS]->failure;      

    std::cout << Utf8PrintfString("Total                 %6d %10d", totalSuccess, totalFailure) << std::endl;
    std::cout << Utf8PrintfString("Total Requests        %6d", totalSuccess + totalFailure) << std::endl;

    std::cout << std::endl;

    }


///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
void RDSStats::WriteToFileBody(int userCount, Utf8String path, std::ofstream& file)
    {
    file << Utf8PrintfString("Create Reality Data   %6d %10d %9d %10d %9d", opStats[OperationType::CREATE_REALITYDATA]->success, opStats[OperationType::CREATE_REALITYDATA]->failure, (int)opStats[OperationType::CREATE_REALITYDATA]->minTime, (int)opStats[OperationType::CREATE_REALITYDATA]->maxTime, (int)opStats[OperationType::CREATE_REALITYDATA]->avgTime) << std::endl;
    file << Utf8PrintfString("Create Relationship   %6d %10d %9d %10d %9d", opStats[OperationType::CREATE_RELATIONSHIP]->success, opStats[OperationType::CREATE_RELATIONSHIP]->failure, (int)opStats[OperationType::CREATE_RELATIONSHIP]->minTime, (int)opStats[OperationType::CREATE_RELATIONSHIP]->maxTime, (int)opStats[OperationType::CREATE_RELATIONSHIP]->avgTime) << std::endl;
    file << Utf8PrintfString("Details               %6d %10d %9d %10d %9d", opStats[OperationType::DETAILS]->success, opStats[OperationType::DETAILS]->failure, (int)opStats[OperationType::DETAILS]->minTime, (int)opStats[OperationType::DETAILS]->maxTime, (int)opStats[OperationType::DETAILS]->avgTime) << std::endl;
    file << Utf8PrintfString("List Reality Data     %6d %10d %9d %10d %9d", opStats[OperationType::LIST_REALITYDATA]->success, opStats[OperationType::LIST_REALITYDATA]->failure, (int)opStats[OperationType::LIST_REALITYDATA]->minTime, (int)opStats[OperationType::LIST_REALITYDATA]->maxTime, (int)opStats[OperationType::LIST_REALITYDATA]->avgTime) << std::endl;
    file << Utf8PrintfString("List Relationship     %6d %10d %9d %10d %9d", opStats[OperationType::LIST_RELATIONSHIP]->success, opStats[OperationType::LIST_RELATIONSHIP]->failure, (int)opStats[OperationType::LIST_RELATIONSHIP]->minTime, (int)opStats[OperationType::LIST_RELATIONSHIP]->maxTime, (int)opStats[OperationType::LIST_RELATIONSHIP]->avgTime) << std::endl;
    file << Utf8PrintfString("Enterprise Stat       %6d %10d %9d %10d %9d", opStats[OperationType::ENTERPRISE_STAT]->success, opStats[OperationType::ENTERPRISE_STAT]->failure, (int)opStats[OperationType::ENTERPRISE_STAT]->minTime, (int)opStats[OperationType::ENTERPRISE_STAT]->maxTime, (int)opStats[OperationType::ENTERPRISE_STAT]->avgTime) << std::endl;
    file << Utf8PrintfString("Change Reality Data   %6d %10d %9d %10d %9d", opStats[OperationType::MODIFY_REALITYDATA]->success, opStats[OperationType::MODIFY_REALITYDATA]->failure, (int)opStats[OperationType::MODIFY_REALITYDATA]->minTime, (int)opStats[OperationType::MODIFY_REALITYDATA]->maxTime, (int)opStats[OperationType::MODIFY_REALITYDATA]->avgTime) << std::endl;
    file << Utf8PrintfString("Delete Relationship   %6d %10d %9d %10d %9d", opStats[OperationType::DELETE_RELATIONSHIP]->success, opStats[OperationType::DELETE_RELATIONSHIP]->failure, (int)opStats[OperationType::DELETE_RELATIONSHIP]->minTime, (int)opStats[OperationType::DELETE_RELATIONSHIP]->maxTime, (int)opStats[OperationType::DELETE_RELATIONSHIP]->avgTime) << std::endl;
    file << Utf8PrintfString("Delete Reality Data   %6d %10d %9d %10d %9d", opStats[OperationType::DELETE_REALITYDATA]->success, opStats[OperationType::DELETE_REALITYDATA]->failure, (int)opStats[OperationType::DELETE_REALITYDATA]->minTime, (int)opStats[OperationType::DELETE_REALITYDATA]->maxTime, (int)opStats[OperationType::DELETE_REALITYDATA]->avgTime) << std::endl;
    file << Utf8PrintfString("AzureAddress          %6d %10d %9d %10d %9d", opStats[OperationType::AZURE_ADDRESS]->success, opStats[OperationType::AZURE_ADDRESS]->failure, (int)opStats[OperationType::AZURE_ADDRESS]->minTime, (int)opStats[OperationType::AZURE_ADDRESS]->maxTime, (int)opStats[OperationType::AZURE_ADDRESS]->avgTime) << std::endl;
    
    file << std::endl << std::endl << "op list:" << std::endl;

    for (Utf8String op : opLog)
        file << op << std::endl;

    file << std::endl << std::endl << "error list:" << std::endl;
    
    file << "List Reality Data:" << std::endl;
    for (Utf8String error : errors[OperationType::LIST_REALITYDATA])
        file << error << std::endl;

    file << "List Relationship:" << std::endl;
    for (Utf8String error : errors[OperationType::LIST_REALITYDATA])
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

    file << "Create Relationship:" << std::endl;
    for (Utf8String error : errors[OperationType::CREATE_RELATIONSHIP])
        file << error << std::endl;

    file << "Delete Relationship:" << std::endl;
    for (Utf8String error : errors[OperationType::DELETE_RELATIONSHIP])
        file << error << std::endl;

    file << "Delete Reality Data:" << std::endl;
    for (Utf8String error : errors[OperationType::DELETE_REALITYDATA])
        file << error << std::endl;
    
    file.close();
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
RDSUser::RDSUser(): User(), m_handshake(AzureHandshake()) {}

RDSUser::RDSUser(int id, Stats* stats) : User(id, stats), m_handshake(AzureHandshake()) {}

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* prepares next request, based on most recent action
//* can be called blindly from outside
//+---------------+---------------+---------------+---------------+---------------+------*/
void RDSUser::DoNextBody(UserManager* owner)
    {
    m_currentOperation = static_cast<int>(rand() % OperationType::last);

    CURL* curl = nullptr;
    if(m_currentOperation == OperationType::DETAILS)
        {
        if(!m_id.empty())
            curl = Details();
        else
            m_currentOperation = OperationType::CREATE_REALITYDATA;
        }
    else if (m_currentOperation == OperationType::AZURE_ADDRESS)
            {
            if (!m_id.empty())
                curl = AzureAddress();
            else
                m_currentOperation = OperationType::CREATE_REALITYDATA;
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
                if (m_linked) // Cannot delete a RD that has relationships
                    {
                    m_currentOperation = OperationType::DELETE_RELATIONSHIP;
                    curl = DeleteRelationship();
                    }
                else
                    curl = DeleteRealityData();
            }
    else if (m_currentOperation == OperationType::LIST_REALITYDATA)
            {
            curl = ListRealityData();
            }    
    else if (m_currentOperation == OperationType::LIST_RELATIONSHIP)
            {
            if (m_id.empty())
                m_currentOperation =  OperationType::CREATE_REALITYDATA;
            else
                curl = ListRelationship();
            }
    else if (m_currentOperation == OperationType::ENTERPRISE_STAT)
            {
            curl = EnterpriseStat();
            }

    if (m_currentOperation == OperationType::CREATE_REALITYDATA)
            {
            if(m_id.empty())
                curl = CreateRealityData();
            else
                {
                if (m_linked) // Cannot delete a RD that has relationships
                    {
                    m_currentOperation = OperationType::DELETE_RELATIONSHIP;
                    curl = DeleteRelationship();
                    }
                else
                    {
                    m_currentOperation = OperationType::DELETE_REALITYDATA;
                    curl = DeleteRealityData();
                    }
                }
            }

    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(m_start);
    if(curl != nullptr)
        owner->SetupCurl(curl, this);
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* prepares next request, based on most recent action
//* can be called blindly from outside
//+---------------+---------------+---------------+---------------+---------------+------*/
void RDSUser::WrapUp(UserManager* owner)
    {
    if (m_id.empty())
        return;

    m_currentOperation = OperationType::DELETE_REALITYDATA;
    CURL* curl = DeleteRealityData();
    
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(m_start);
    if (curl != nullptr)
        owner->SetupCurl(curl, this);
    }

CURL* RDSUser::ListRealityData()
    {
    RealityDataListByOrganizationPagedRequest organizationReq = RealityDataListByOrganizationPagedRequest("", 0, 25);

    organizationReq.SetFilter(RealityDataFilterCreator::FilterBySize(100, 100000));

    m_correspondance.response.clear();
    m_correspondance.req.url = organizationReq.GetHttpRequestString();
    m_correspondance.req.headers = organizationReq.GetRequestHeaders();
    m_correspondance.req.payload = organizationReq.GetRequestPayload();

    m_correspondance.id = m_stats->LogRequest("List Reality Data");

    return WSGRequest::GetInstance().PrepareRequest(organizationReq, m_correspondance.response, false, nullptr);
    }

void RDSUser::ValidateListRealityData(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    m_stats->InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "instances") == RequestStatus::OK || m_correspondance.response.status == RequestStatus::LASTPAGE),
        activeUsers);
    }

CURL* RDSUser::ListRelationship()
    {
    RealityDataProjectRelationshipByProjectIdPagedRequest listReq = RealityDataProjectRelationshipByProjectIdPagedRequest(m_id);

    m_correspondance.response.clear();
    m_correspondance.req.url = listReq.GetHttpRequestString();
    m_correspondance.req.headers = listReq.GetRequestHeaders();
    m_correspondance.req.payload = listReq.GetRequestPayload();

    m_correspondance.id = m_stats->LogRequest("List relationship");

    return WSGRequest::GetInstance().PrepareRequest(listReq, m_correspondance.response, false, nullptr);
    }

void RDSUser::ValidateListRelationship(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    m_stats->InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "instances") == RequestStatus::OK || m_correspondance.response.status == RequestStatus::LASTPAGE),
        activeUsers);
    }

CURL* RDSUser::EnterpriseStat()
    {
    m_correspondance.response.clear();
    RealityDataEnterpriseStatRequest enterpriseReq = RealityDataEnterpriseStatRequest("");
    m_correspondance.req.url = enterpriseReq.GetHttpRequestString();
    m_correspondance.req.headers = enterpriseReq.GetRequestHeaders();
    m_correspondance.req.payload = enterpriseReq.GetRequestPayload();

    m_correspondance.id = m_stats->LogRequest("stat");

    return WSGRequest::GetInstance().PrepareRequest(enterpriseReq, m_correspondance.response, false, nullptr);
    }

void RDSUser::ValidateEnterpriseStat(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    m_stats->InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "instances") == RequestStatus::OK),
        activeUsers);
    }

CURL* RDSUser::Details()
    {
    m_correspondance.response.clear();
    
    RealityDataByIdRequest idReq = RealityDataByIdRequest(m_id);
    m_correspondance.req.url = idReq.GetHttpRequestString();
    m_correspondance.req.headers = idReq.GetRequestHeaders();
    m_correspondance.req.payload = idReq.GetRequestPayload();

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("RealityData for %s", m_id));

    return WSGRequest::GetInstance().PrepareRequest(idReq, m_correspondance.response, false, nullptr);
        
    return nullptr;
    }

void RDSUser::ValidateDetails(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    m_stats->InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "instances") == RequestStatus::OK),
        activeUsers);
    }

CURL* RDSUser::AzureAddress()
    {
    m_handshake = AzureHandshake(m_id, false);

    m_correspondance.response = RawServerResponse();
    m_correspondance.req.url = m_handshake.GetHttpRequestString();
    m_correspondance.req.headers = m_handshake.GetRequestHeaders();
    m_correspondance.req.payload = m_handshake.GetRequestPayload();

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("azure address for %s -  user %d", m_id, m_userId));

    return WSGRequest::GetInstance().PrepareRequest(m_handshake, m_correspondance.response, false, nullptr);
    }

void RDSUser::ValidateAzureAddress(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    m_stats->InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "instances") == RequestStatus::OK),
        activeUsers);
    }

CURL* RDSUser::CreateRealityData()
    {
    BeAssert(m_id.empty());

    bmap<RealityDataField, Utf8String> properties = bmap<RealityDataField, Utf8String>();
    
    properties.Insert(RealityDataField::Name, "Load Test (ERASE)");

    properties.Insert(RealityDataField::Classification, "Model");

    properties.Insert(RealityDataField::Type, "3DTiles");

    properties.Insert(RealityDataField::Visibility, "ENTERPRISE");

    RealityDataCreateRequest createRequest = RealityDataCreateRequest("", RealityDataServiceUpload::PackageProperties(properties));
    m_correspondance.response.clear();
    m_correspondance.req.url = createRequest.GetHttpRequestString();
    m_correspondance.req.headers = createRequest.GetRequestHeaders();
    m_correspondance.req.payload = createRequest.GetRequestPayload();

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("Create Reality Data for user %d", m_userId));

    return WSGRequest::GetInstance().PrepareRequest(createRequest, m_correspondance.response, false, nullptr);
    }

void RDSUser::ValidateCreateRealityData(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    m_stats->InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "changedInstance") == RequestStatus::OK),
        activeUsers);

    if(m_correspondance.response.status == RequestStatus::OK)
        m_id = instances["changedInstance"]["instanceAfterChange"]["instanceId"].asString();

    m_linked = false;
    }

CURL* RDSUser::ModifyRealityData()
    {
    bmap<RealityDataField, Utf8String> props = bmap<RealityDataField, Utf8String>();
    Utf8String propertyString = "\"Listable\" : true";

    RealityDataChangeRequest changeReq = RealityDataChangeRequest(m_id, propertyString);

    m_correspondance.response.clear();
    m_correspondance.req.url = changeReq.GetHttpRequestString();
    m_correspondance.req.headers = changeReq.GetRequestHeaders();
    m_correspondance.req.payload = changeReq.GetRequestPayload();

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("Change Reality Data for user %d", m_userId));

    return WSGRequest::GetInstance().PrepareRequest(changeReq, m_correspondance.response, false, nullptr);
    }

void RDSUser::ValidateModifyRealityData(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    m_stats->InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "changedInstance") == RequestStatus::OK),
        activeUsers);
    }

CURL* RDSUser::CreateRelationship()
    {
    BeAssert(!m_linked);

    RealityDataRelationshipCreateRequest relReq = RealityDataRelationshipCreateRequest(m_id, RealityDataService::GetProjectId());

    m_correspondance.response.clear();
    m_correspondance.req.url = relReq.GetHttpRequestString();
    m_correspondance.req.headers = relReq.GetRequestHeaders();
    m_correspondance.req.payload = relReq.GetRequestPayload();

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("Create Relationship for user %d - ID:%s", m_userId, m_id));

    return WSGRequest::GetInstance().PrepareRequest(relReq, m_correspondance.response, false, nullptr);
    }

void RDSUser::ValidateCreateRelationship(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    m_stats->InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "changedInstance") == RequestStatus::OK),
        activeUsers);

    if(m_correspondance.response.status == RequestStatus::OK)
        m_linked = true;
    }

CURL* RDSUser::DeleteRelationship()
    {
    BeAssert(m_linked);

    RealityDataRelationshipDelete relReq = RealityDataRelationshipDelete(m_id, RealityDataService::GetProjectId());

    m_correspondance.response.clear();
    m_correspondance.req.url = relReq.GetHttpRequestString();
    m_correspondance.req.headers = relReq.GetRequestHeaders();
    m_correspondance.req.payload = relReq.GetRequestPayload();

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("DeleteRelationship for user %d- ID:%s", m_userId, m_id));

    return WSGRequest::GetInstance().PrepareRequest(relReq, m_correspondance.response, false, nullptr);
    }

void RDSUser::ValidateDeleteRelationship(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    m_stats->InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "changedInstance") == RequestStatus::OK),
        activeUsers);

    if (m_correspondance.response.status == RequestStatus::OK)
        m_linked = false;
    }

CURL* RDSUser::DeleteRealityData()
    {
    BeAssert(!m_linked);
    RealityDataDelete realityDataReq = RealityDataDelete(m_id);
    m_correspondance.response.clear();
    m_correspondance.req.url = realityDataReq.GetHttpRequestString();
    m_correspondance.req.headers = realityDataReq.GetRequestHeaders();
    m_correspondance.req.payload = realityDataReq.GetRequestPayload();

    m_correspondance.id = m_stats->LogRequest(Utf8PrintfString("Delete for user %d - ID:%s", m_userId, m_id));

    return WSGRequest::GetInstance().PrepareRequest(realityDataReq, m_correspondance.response, false, nullptr);
    }

void RDSUser::ValidateDeleteRealityData(int activeUsers)
    {
    Json::Value instances(Json::objectValue);

    m_stats->InsertStats(this,
        (m_correspondance.response.ValidateJSONResponse(instances, "changedInstance") == RequestStatus::OK),
        activeUsers);

    if (m_correspondance.response.status == RequestStatus::OK)
        {
        m_id = "";
        m_linked = false;
        }
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//* automatically call the proper validation function for the last executed operation
//+---------------+---------------+---------------+---------------+---------------+------*/
void RDSUser::ValidatePrevious(int activeUsers)
    {
    if(m_correspondance.response.curlCode != CURLE_OK)
        {
        m_stats->InsertStats(this, false, activeUsers);
        return;
        }
    switch (m_currentOperation)
        {
        case OperationType::DETAILS:
            return ValidateDetails(activeUsers);
        case OperationType::AZURE_ADDRESS:
            return ValidateAzureAddress(activeUsers);
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
        case OperationType::LIST_RELATIONSHIP:
            return ValidateListRelationship(activeUsers);
        case OperationType::CREATE_REALITYDATA:
            return ValidateCreateRealityData(activeUsers);
        case OperationType::ENTERPRISE_STAT:
            return ValidateEnterpriseStat(activeUsers);
        }
    }

///*---------------------------------------------------------------------------------**//**
//* @bsifunction                                    Spencer Mason                   4/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
int main(int argc, char* argv[])
    {
    LoadTester tester = LoadTester();
    RDSRPS rps = RDSRPS();
    RDSStats stats = RDSStats(&rps);
    Utf8String projectId;

    LoadTester::SetupStaticClasses(&stats, &rps);
    if (tester.Main(argc, argv))
        return 0;

    char* substringPosition;
    for (int i = 0; i < argc; ++i)
        {
        if (strstr(argv[i], "-o:") || strstr(argv[i], "--project:"))
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            projectId = Utf8String(substringPosition);
            }
        }

    if (projectId.empty())
        {
        std::cout << "  -o, --project  A project id to an existing project for which you have required permissions is mandatory" << std::endl;
        }

    std::queue<User*> inactiveUsers;

    if(!tester.trickle)
        {
        for(int i = 0; i < tester.userManager.m_userCount; i++)
            {
            tester.userManager.users.push_back(new RDSUser(i, &stats));
            }
        }
    else
        {
        tester.userManager.users.push_back(new RDSUser(0, &stats)); //start with one
        for (int i = 1; i < tester.userManager.m_userCount; i++)
            {
            inactiveUsers.push(new RDSUser(i, &stats)); //feed the rest to the Dispatcher
            }
        }

    if (tester.m_serverType == RealityPlatform::CONNECTServerType::PROD)
        RealityDataService::SetServerComponents("connect-realitydataservices.bentley.com", "2.4", "S3MXECPlugin--Server", "S3MX");
    else if (tester.m_serverType == RealityPlatform::CONNECTServerType::QA)
        RealityDataService::SetServerComponents("qa-connect-realitydataservices.bentley.com", "2.4", "S3MXECPlugin--Server", "S3MX");
    else // (tester.m_serverType == RealityPlatform::CONNECTServerType::DEV)
        RealityDataService::SetServerComponents("dev-realitydataservices-eus.cloudapp.net", "2.4", "S3MXECPlugin--Server", "S3MX");

    tester.SetupInactiveUsers(inactiveUsers);

    RealityDataService::SetProjectId(projectId); // 0ae1fcca-ead6-4e94-b83f-38d25db1b16e

    tester.Main2(100);
    stats.WriteToFile(tester.userManager.m_userCount, tester.path);
    }
