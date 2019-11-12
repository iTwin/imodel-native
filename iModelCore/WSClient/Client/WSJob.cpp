/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/WSJob.h>
#include <WebServices/Client/Response/WSObjectsReaderV2.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+--------------------------------------------------------------------------------------*/
WSJob::WSJob(Http::Response response, Utf8StringCR jobOriginUrl) : WSJob(response)
    {
    m_status = response.GetConnectionStatus();
    m_url = jobOriginUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WSJob::WSJob(Http::Response response) 
    {
    auto rapidJson = std::make_shared<rapidjson::Document>();
    bool fail = rapidJson->Parse<0>(response.GetBody().AsString().c_str()).HasParseError();
    BeAssert(!fail && "Check json string");
    m_reader = WSObjectsReaderV2::Create();
    m_reader->ReadInstances(rapidJson);

    Validate();

    m_status = response.GetConnectionStatus();
    m_url = response.GetEffectiveUrl();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void WSJob::Validate()
    {
    m_isValid = false;
    m_hasResponseBody = false;

    if (1 != m_reader->GetInstanceCount())
        return;

    if (!m_reader->GetInstance(0).IsValid())
        return;

    if (!m_reader->GetInstance(0).GetProperties().HasMember(WSJobResponse_Status))
        return;

    if (!m_reader->GetInstance(0).GetProperties()[WSJobResponse_Status].IsString())
        return;

    if (!m_reader->GetInstance(0).GetProperties().HasMember(WSJobResponse_ScheduleTime))
        return;

    if (!m_reader->GetInstance(0).GetProperties()[WSJobResponse_ScheduleTime].IsString())
        return;

    DateTime dateTimeOut;
    auto str = m_reader->GetInstance(0).GetProperties()[WSJobResponse_ScheduleTime].GetString();
    if (BentleyStatus::SUCCESS != DateTime::FromString(dateTimeOut, str))
        return;

    m_isValid = true;

    if (m_reader->GetInstance(0).GetProperties().HasMember(WSJobResponse_Content) && 
        !m_reader->GetInstance(0).GetProperties()[WSJobResponse_Content].IsString())
            return;

    if (!m_reader->GetInstance(0).GetProperties().HasMember(WSJobResponse_Headers))
        return;

    if (!m_reader->GetInstance(0).GetProperties()[WSJobResponse_Headers].IsString())
        return;

    if (!m_reader->GetInstance(0).GetProperties().HasMember(WSJobResponse_StatusCode))
        return;

    if (!m_reader->GetInstance(0).GetProperties()[WSJobResponse_StatusCode].IsInt())
        return;

    m_hasResponseBody = true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
WSJob::Status WSJob::GetErrorIdFromString(Utf8StringCR errorIdString)
    {
    static std::map<Utf8String, WSJob::Status> map =
        {
            {"Failed", WSJob::Status::Failed},
            {"NotStarted", WSJob::Status::NotStarted},
            {"Running", WSJob::Status::Running},
            {"Succeeded", WSJob::Status::Succeeded},
        };

    auto it = map.find(errorIdString);
    if (it != map.end())
        return it->second;

    return WSJob::Status::Failed;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+--------------------------------------------------------------------------------------*/
bool WSJob::IsValid() const
    {
    return m_isValid;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+--------------------------------------------------------------------------------------*/
bool WSJob::HasResponseContent() const
    {
    return m_hasResponseBody;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+--------------------------------------------------------------------------------------*/
WSJob::Status WSJob::GetStatus() const
    {
    if (!m_isValid)
        return WSJob::Status::Failed;

    return GetErrorIdFromString(m_reader->GetInstance(0).GetProperties()[WSJobResponse_Status].GetString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+--------------------------------------------------------------------------------------*/
DateTime WSJob::GetScheduleTime() const
    {
    DateTime dateTimeOut;

    if (!m_isValid)
        return dateTimeOut;

    auto str = m_reader->GetInstance(0).GetProperties()[WSJobResponse_ScheduleTime].GetString();
    DateTime::FromString(dateTimeOut, str);

    return dateTimeOut;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+--------------------------------------------------------------------------------------*/
Http::Response WSJob::GetResponse() const
    {
    if (!m_hasResponseBody)
        return Http::Response();

    const char* responseContent = "";
    const char* responseHeaders = "";
    int responseStatus = 0;

    if (m_reader->GetInstance(0).GetProperties().HasMember(WSJobResponse_Content))
        responseContent = m_reader->GetInstance(0).GetProperties()[WSJobResponse_Content].GetString();

    responseHeaders = m_reader->GetInstance(0).GetProperties()[WSJobResponse_Headers].GetString();
    responseStatus = m_reader->GetInstance(0).GetProperties()[WSJobResponse_StatusCode].GetInt();

    return Http::Response(HttpStatus(responseStatus), m_url.c_str(), responseHeaders, responseContent);
    }
