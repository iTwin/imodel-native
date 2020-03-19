/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeFwk.h>
#include <BeHttp/HttpBody.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <DgnPlatform/DgnProgressMeter.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

namespace {
struct BroadcasterBase
    {
    Utf8String m_url;
    Utf8String m_jobRunCorrelationId;
    Utf8String m_jobRequestId;
    uint32_t m_timeOfLastPost {};
    uint32_t m_interval = 1000;
    bool m_isRequestInFlight{};

    protected:
    void PostRequest(JsonValueCR);

    public:
    void SetUrl(Utf8StringCR url) {m_url = url;}
    void SetCorrelationId(Utf8String correlationId) { m_jobRunCorrelationId = correlationId; }
    void SetJobRequestId(Utf8String jobRequestId) { m_jobRequestId = jobRequestId; }
    bool HasUrl() const {return !m_url.empty();}
    void SetInterval(uint32_t i) {m_interval=i;}
    bool IsTooSoonForNextMessage() const;
    void SetTimeOfLastPost() {m_timeOfLastPost = BeTimeUtilities::QueryMillisecondsCounterUInt32();}
    };

struct BroadcastProgressMeter : DgnProgressMeter, BroadcasterBase
    {
    Utf8String m_phaseName;
    Utf8String m_stepName;
    Utf8String m_taskName;
    uint32_t m_totalPhases{}, m_totalSteps{}, m_totalTasks{};
    uint32_t m_phasesRemaining {};
    uint32_t m_spinCount {};

    void _Hide() override;
    Abort _ShowProgress() override;
    void _SetCurrentStepName(Utf8CP stepName) override;
    void _SetCurrentTaskName(Utf8CP taskName) override;
    void _AddSteps(uint32_t numSteps) override {m_totalSteps += numSteps; DgnProgressMeter::_AddSteps(numSteps); }
    void _AddTasks(uint32_t numTasksToAdd) override {m_totalTasks += numTasksToAdd; DgnProgressMeter::_AddTasks(numTasksToAdd);}

    void PostProgressMessage();

    public:

    void AddPhases(uint32_t numPhases) {m_totalPhases += numPhases; m_phasesRemaining += numPhases;}
    void SetCurrentPhaseName(Utf8StringCR);
    };

struct StatusMessageBroadcaster : BroadcasterBase
    {
    void PostMessage(Utf8StringCR msg, Utf8StringCR details, bool isError);
    };
}

static StatusMessageBroadcaster s_statusMessages;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool BroadcasterBase::IsTooSoonForNextMessage() const
    {
    if (m_isRequestInFlight)
        return true;

    return ((BeTimeUtilities::QueryMillisecondsCounterUInt32()- m_timeOfLastPost) < m_interval);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void BroadcasterBase::PostRequest(JsonValueCR json)
    {
    if (m_url.empty())
        return;

    if (m_isRequestInFlight)    // If server is not responding, don't allow unacknowledged requests to pile up!
        return; // TODO It would be nice if I could cancel the outstanding request. This should supercede it.

    Http::HttpClient client;
    Http::Request request = client.CreatePostRequest(m_url);

    HttpStringBodyPtr requestBody = HttpStringBody::Create(Json::FastWriter().write(json));
    request.SetRequestBody(requestBody);

    m_isRequestInFlight = true;
    request.Perform().then([&] (Http::Response const& response) {
        m_isRequestInFlight = false;
    });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void StatusMessageBroadcaster::PostMessage(Utf8StringCR msg, Utf8StringCR details, bool isError)
    {
    if (IsTooSoonForNextMessage())
        return;

    SetTimeOfLastPost();

    Json::Value json(Json::objectValue);
    json["messageType"] = "status";
    json["message"] = msg.c_str();
    if (!details.empty())
        json["details"] = details.c_str();
    if (isError)
        json["isError"] = true;

    PostRequest(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::PostStatusMessage(Utf8StringCR msg, Utf8StringCR details, bool isError)
    {
    if (m_jobEnvArgs.m_statusMessageSinkUrl.empty())
        return;

    if (!s_statusMessages.HasUrl())
        {
        s_statusMessages.SetUrl(m_jobEnvArgs.m_statusMessageSinkUrl);
        s_statusMessages.SetInterval(m_jobEnvArgs.m_statusMessageInterval);
        }

    s_statusMessages.PostMessage(msg, details, isError);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/19
+---------------+---------------+---------------+---------------+---------------+------*/
static int pct(uint32_t remaining, uint32_t total)
    {
    if (total == 0)
        return 0;
    uint32_t done = (total - (remaining + 1)); // tricky: "remaining" actually includes the step that is in progress. So, that step is not done yet.
    if (done > total)
        return 100;
    return (done * 100) / total;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void BroadcastProgressMeter::PostProgressMessage()
    {
    if (IsTooSoonForNextMessage())
        return;

    SetTimeOfLastPost();

    Json::Value json(Json::objectValue);
    json["jobRequestId"] = m_jobRequestId;
    json["jobRunCorrelationId"] = m_jobRunCorrelationId;
    json["messageType"] = "progress";
    json["phase"] = m_phaseName.c_str();
    json["step"] = m_stepName.c_str();
    json["task"] = m_taskName.c_str();
    json["phasesPct"] = pct(m_phasesRemaining, m_totalPhases);
    json["stepsPct"] = pct(m_stepsRemaining, m_totalSteps);
    json["tasksPct"] = pct(m_tasksRemaining, m_totalTasks);
    json["lastUpdateTime"] = m_timeOfLastPost;
    json["spinCount"] = (int)m_spinCount;
    json["phasesCount"] = m_totalPhases;

    PostRequest(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void BroadcastProgressMeter::_Hide()
    {
    // TODO
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
BroadcastProgressMeter::Abort BroadcastProgressMeter::_ShowProgress()
    {
    ++m_spinCount;
    PostProgressMessage();
    return ABORT_No;    // TODO
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void BroadcastProgressMeter::SetCurrentPhaseName(Utf8StringCR newName)
    {
    --m_phasesRemaining;

    m_phaseName = newName;
    m_stepName.clear();
    m_taskName.clear();
    m_totalSteps = 0;
    m_totalTasks = 0;
    m_spinCount=0;
    PostProgressMessage();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void BroadcastProgressMeter::_SetCurrentStepName(Utf8CP newName)
    {
    if (newName && m_stepName == newName)
        return;

    DgnProgressMeter::_SetCurrentStepName(newName);

    m_stepName.AssignOrClear(newName);

    m_taskName.clear();
    m_totalTasks = 0;
    m_spinCount=0;
    PostProgressMessage();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void BroadcastProgressMeter::_SetCurrentTaskName(Utf8CP newName)
    {
    if (newName && m_taskName == newName)
        return;

    DgnProgressMeter::_SetCurrentTaskName(newName);

    m_taskName.AssignOrClear(newName);
    m_spinCount=0;
    PostProgressMessage();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SetupProgressMeter()
    {
    static PrintfProgressMeter s_printfMeter;
    static BroadcastProgressMeter s_broadcastMeter;
    if (!m_jobEnvArgs.m_statusMessageSinkUrl.empty())
        {
        s_broadcastMeter.SetUrl(m_jobEnvArgs.m_statusMessageSinkUrl);
        s_broadcastMeter.SetInterval(m_jobEnvArgs.m_statusMessageInterval);
        s_broadcastMeter.SetJobRequestId(m_jobEnvArgs.m_jobRequestId);
        s_broadcastMeter.SetCorrelationId(m_jobEnvArgs.m_jobRunCorrelationId);
        T_HOST.SetProgressMeter(&s_broadcastMeter);
        }
    else
        {
        T_HOST.SetProgressMeter(&s_printfMeter); 
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProgressMeter& iModelBridgeFwk::GetProgressMeter() const
    {
    static NopProgressMeter s_nopMeter;
    auto meter = T_HOST.GetProgressMeter();
    return meter? *meter: s_nopMeter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::AddPhases(uint32_t count)
    {
    auto p = dynamic_cast<BroadcastProgressMeter*>(T_HOST.GetProgressMeter());
    if (nullptr == p)
        return;
    p->AddPhases(count);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeFwk::SetCurrentPhaseName(Utf8StringCR newName)
    {
    auto p = dynamic_cast<BroadcastProgressMeter*>(T_HOST.GetProgressMeter());
    if (nullptr == p)
        return;
    p->SetCurrentPhaseName(newName);
    }
