/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "../../RulesEngineTypes.h"

struct ThreadLocalScopeStorage
    {
    std::weak_ptr<Diagnostics::Scope> weak;
    Diagnostics::Scope* raw;
    ThreadLocalScopeStorage() : raw(nullptr) {}
    };
static BeThreadLocalStorage s_currentScopeStorage([](void* ptr)
    {
    delete static_cast<ThreadLocalScopeStorage*>(ptr);
    });
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ThreadLocalScopeStorage& GetThreadLocalScopeStorage()
    {
    void* ptr = s_currentScopeStorage.GetValueAsPointer();
    if (nullptr != ptr)
        return *reinterpret_cast<ThreadLocalScopeStorage*>(ptr);

    ThreadLocalScopeStorage* storage = new ThreadLocalScopeStorage();
    s_currentScopeStorage.SetValueAsPointer(storage);
    return *storage;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP GetDiagnosticsCategoryName(DiagnosticsCategory category)
    {
    switch (category)
        {
        case DiagnosticsCategory::Default: return LOGGER_NAMESPACE_ECPRESENTATION;
        case DiagnosticsCategory::Serialization: return LOGGER_NAMESPACE_ECPRESENTATION_SERIALIZATION;
        case DiagnosticsCategory::Tasks: return LOGGER_NAMESPACE_ECPRESENTATION_TASKS;
        case DiagnosticsCategory::Connections: return LOGGER_NAMESPACE_ECPRESENTATION_CONNECTIONS;

        case DiagnosticsCategory::Performance: return LOGGER_NAMESPACE_ECPRESENTATION_PERFORMANCE;

        case DiagnosticsCategory::Rules: return LOGGER_NAMESPACE_ECPRESENTATION_RULES;
        case DiagnosticsCategory::RulesetVariables: return LOGGER_NAMESPACE_ECPRESENTATION_RULESET_VARIABLES;
        case DiagnosticsCategory::ECExpressions: return LOGGER_NAMESPACE_ECPRESENTATION_ECEXPRESSIONS;

        case DiagnosticsCategory::Hierarchies: return LOGGER_NAMESPACE_ECPRESENTATION_HIERARCHIES;
        case DiagnosticsCategory::HierarchiesCache: return LOGGER_NAMESPACE_ECPRESENTATION_HIERARCHIES_CACHE;

        case DiagnosticsCategory::Content: return LOGGER_NAMESPACE_ECPRESENTATION_CONTENT;

        case DiagnosticsCategory::Update: return LOGGER_NAMESPACE_ECPRESENTATION_UPDATE;
        case DiagnosticsCategory::HierarchiesUpdate: return LOGGER_NAMESPACE_ECPRESENTATION_UPDATE_HIERARCHIES;
        case DiagnosticsCategory::ContentUpdate: return LOGGER_NAMESPACE_ECPRESENTATION_UPDATE_CONTENT;
        }
    BeAssert(false);
    return LOGGER_NAMESPACE_ECPRESENTATION;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP GetSeverityString(NativeLogging::SEVERITY severity)
    {
    switch (severity)
        {
        case LOG_FATAL: return "fatal";
        case LOG_ERROR: return "error";
        case LOG_WARNING: return "warning";
        case LOG_INFO: return "info";
        case LOG_TRACE: return "trace";
        }
    BeAssert(false);
    return "none";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Diagnostics::Message::Message(DiagnosticsCategory category, NativeLogging::SEVERITY const* devSeverity, NativeLogging::SEVERITY const* editorSeverity, Utf8String message)
    : m_category(category), m_message(message), m_timestamp(BeTimeUtilities::GetCurrentTimeAsUnixMillis())
    {
    m_devSeverityStr = devSeverity ? GetSeverityString(*devSeverity) : nullptr;
    m_editorSeverityStr = editorSeverity ? GetSeverityString(*editorSeverity) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document Diagnostics::Message::_BuildJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    rapidjson::Value severityJson(rapidjson::kObjectType);
    if (m_devSeverityStr)
        severityJson.AddMember("dev", rapidjson::StringRef(m_devSeverityStr), json.GetAllocator());
    if (m_editorSeverityStr)
        severityJson.AddMember("editor", rapidjson::StringRef(m_editorSeverityStr), json.GetAllocator());
    json.AddMember("severity", severityJson, json.GetAllocator());
    json.AddMember("category", rapidjson::StringRef(GetDiagnosticsCategoryName(m_category)), json.GetAllocator());
    json.AddMember("message", rapidjson::Value(m_message.c_str(), json.GetAllocator()), json.GetAllocator());
    json.AddMember("timestamp", m_timestamp, json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Diagnostics::Scope::Holder Diagnostics::Scope::ResetAndCreate(Utf8String name, Options options)
    {
    return std::make_shared<Scope>(nullptr, name, std::make_shared<Options>(options));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Diagnostics::Scope::Holder Diagnostics::Scope::Create(Utf8String name, Options options)
    {
    auto parentScope = Diagnostics::GetCurrentScope().lock();
    auto scope = std::make_shared<Scope>(parentScope, name, std::make_shared<Options>(options));
    if (parentScope)
        parentScope->AddItem(scope);
    return scope;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Diagnostics::Scope::Holder Diagnostics::Scope::Create(Utf8String name)
    {
    auto parentScope = Diagnostics::GetCurrentScope().lock();
    auto scope = std::make_shared<Scope>(parentScope, name, parentScope ? parentScope->m_options : nullptr);
    if (parentScope)
        parentScope->AddItem(scope);
    return scope;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Diagnostics::Scope::Scope(std::shared_ptr<Scope> parentScope, Utf8String name, std::shared_ptr<Options> options)
    : enable_shared_from_this(), m_parentScope(parentScope), m_name(name), m_options(options),
    m_threadId((uint64_t)BeThreadUtilities::GetCurrentThreadId()), m_start(BeTimeUtilities::GetCurrentTimeAsUnixMillis()),
    m_end(0), m_isFinalized(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::Scope::Attach()
    {
    Diagnostics::SetCurrentScope(shared_from_this());
    m_end = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::Scope::Detach()
    {
    m_end = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    Diagnostics::SetCurrentScope(m_parentScope.lock());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Diagnostics::Options const& Diagnostics::Scope::GetOptions() const
    {
    if (m_options)
        return *m_options;

    static Diagnostics::Options s_empty;
    return s_empty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document Diagnostics::Scope::_BuildJson(rapidjson::Document::AllocatorType* allocator) const
    {
    BeMutexHolder lock(m_mutex);

    uint64_t duration = GetElapsedTime();

    rapidjson::Document json(allocator);
    json.SetObject();

    rapidjson::Value logsJson(rapidjson::kArrayType);
    for (auto const& logItem : m_logItems)
        {
        rapidjson::Document itemJson = logItem->BuildJson(&json.GetAllocator());
        if (!itemJson.IsNull())
            logsJson.PushBack(itemJson, json.GetAllocator());
        }

    bool hasLogs = !logsJson.Empty();
    if (hasLogs)
        json.AddMember("logs", logsJson, json.GetAllocator());

    bool meetsDurationExpectation = (GetOptions().GetMinimumDuration().IsValid() && duration >= GetOptions().GetMinimumDuration().Value());
    if (meetsDurationExpectation || hasLogs)
        {
        json.AddMember("scopeCreateTimestamp", m_start, json.GetAllocator());
        json.AddMember("duration", duration, json.GetAllocator());

        rapidjson::Document attributesJson(&json.GetAllocator());
        attributesJson.SetObject();
        for (auto const& attribute : m_arrayAttributes)
            {
            rapidjson::Value arrayJson(rapidjson::kArrayType);
            for (auto const& value : *attribute.second)
                arrayJson.PushBack(rapidjson::Value(value.c_str(), json.GetAllocator()), json.GetAllocator());
            if (!arrayJson.Empty())
                attributesJson.AddMember(rapidjson::Value(attribute.first, json.GetAllocator()), arrayJson, json.GetAllocator());
            }
        if (attributesJson.MemberCount() != 0)
            json.AddMember("attributes", attributesJson, json.GetAllocator());
        }

    if (0 == json.MemberCount())
        return rapidjson::Document(rapidjson::kNullType);

    json.AddMember("scope", rapidjson::Value(m_name.c_str(), json.GetAllocator()), json.GetAllocator());

    m_isFinalized = true;

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::Scope::AddMessage(DiagnosticsCategory category, NativeLogging::SEVERITY const* devSeverity, NativeLogging::SEVERITY const* editorSeverity, Utf8String msg)
    {
    if (devSeverity && *devSeverity < GetOptions().GetDevSeverity())
        devSeverity = nullptr;

    if (editorSeverity && *editorSeverity < GetOptions().GetEditorSeverity())
        editorSeverity = nullptr;

    // skip scope logging if severity is lower than what's configured for the scope
    if (!devSeverity && !editorSeverity)
        return;

    AddItem(std::make_shared<Message>(category, devSeverity, editorSeverity, msg));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::Scope::AddItem(std::shared_ptr<ILogItem> item)
    {
    BeMutexHolder lock(m_mutex);
    if (m_isFinalized)
        {
        // the scope is already finalized, we shouldn't be adding anything into it anymore
        return;
        }
    m_logItems.push_back(item);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::Scope::Log(DiagnosticsCategory category, NativeLogging::SEVERITY devSeverity, NativeLogging::SEVERITY editorSeverity, Utf8String msg)
    {
    AddMessage(category, &devSeverity, &editorSeverity, msg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::Scope::DevLog(DiagnosticsCategory category, NativeLogging::SEVERITY severity, Utf8String msg)
    {
    AddMessage(category, &severity, nullptr, msg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::Scope::EditorLog(DiagnosticsCategory category, NativeLogging::SEVERITY severity, Utf8String msg)
    {
    AddMessage(category, nullptr, &severity, msg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::Scope::SetCapturedAttributes(bvector<Utf8CP> const& attributes)
    {
    BeMutexHolder lock(m_mutex);
    m_capturedAttributes = bset<Utf8CP>(attributes.begin(), attributes.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::Scope::AddValueToArrayAttribute(Utf8CP name, Utf8String value, bool unique)
    {
    BeMutexHolder lock(m_mutex);
    if (m_capturedAttributes.find(name) == m_capturedAttributes.end())
        {
        auto parentScope = m_parentScope.lock();
        if (parentScope)
            parentScope->AddValueToArrayAttribute(name, value, unique);
        return;
        }

    auto it = m_arrayAttributes.find(name);
    if (it == m_arrayAttributes.end())
        it = m_arrayAttributes.insert({ name, std::make_shared<bvector<Utf8String>>(bvector<Utf8String>{}) }).first;

    bvector<Utf8String>& values = *it->second;

    if (unique && ContainerHelpers::Contains(values, value))
        return;

    values.push_back(value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::SetCurrentScope(std::shared_ptr<Diagnostics::Scope> scope)
    {
    GetThreadLocalScopeStorage().weak = scope;
    GetThreadLocalScopeStorage().raw = scope.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::weak_ptr<Diagnostics::Scope> Diagnostics::GetCurrentScope()
    {
    return GetThreadLocalScopeStorage().weak;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Diagnostics::Scope* Diagnostics::GetCurrentScopeRaw()
    {
    return GetThreadLocalScopeStorage().raw;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsDiagnosticsCategoryEnabledForNativeLogger(DiagnosticsCategory category, NativeLogging::SEVERITY severity)
    {
    // note: we get A LOT of trace messages - don't want to emit those to native logger (even the check if they're enabled gets expensive)
    return severity > NativeLogging::LOG_TRACE
        && NativeLogging::CategoryLogger(GetDiagnosticsCategoryName(category)).isSeverityEnabled(severity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TCallable> static void WithDiagnosticsScope(TCallable cb)
    {
    auto scope = Diagnostics::GetCurrentScope().lock();
    if (scope)
        cb(*scope);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::Log(DiagnosticsCategory category, NativeLogging::SEVERITY devSeverity, NativeLogging::SEVERITY editorSeverity, Utf8String msg)
    {
    // pass dev logs to native logger (messages of severity lower than configured will be dropped)
    if (IsDiagnosticsCategoryEnabledForNativeLogger(category, devSeverity))
        NativeLogging::CategoryLogger(GetDiagnosticsCategoryName(category)).message(devSeverity, msg.c_str());

    WithDiagnosticsScope([&](auto& scope)
        {
        scope.Log(category, devSeverity, editorSeverity, std::move(msg));
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::DevLog(DiagnosticsCategory category, NativeLogging::SEVERITY severity, Utf8String msg)
    {
    // pass dev logs to native logger (messages of severity lower than configured will be dropped)
    if (IsDiagnosticsCategoryEnabledForNativeLogger(category, severity))
        NativeLogging::CategoryLogger(GetDiagnosticsCategoryName(category)).message(severity, msg.c_str());

    WithDiagnosticsScope([&](auto& scope)
        {
        scope.DevLog(category, severity, std::move(msg));
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::EditorLog(DiagnosticsCategory category, NativeLogging::SEVERITY severity, Utf8String msg)
    {
    WithDiagnosticsScope([&](auto& scope)
        {
        scope.EditorLog(category, severity, std::move(msg));
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::SetCapturedAttributes(bvector<Utf8CP> const& attributes)
    {
    WithDiagnosticsScope([&](auto& scope)
        {
        scope.SetCapturedAttributes(attributes);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Diagnostics::AddValueToArrayAttribute(Utf8CP name, Utf8String value, bool unique)
    {
    WithDiagnosticsScope([&](auto& scope)
        {
        scope.AddValueToArrayAttribute(name, value, unique);
        });
    }
