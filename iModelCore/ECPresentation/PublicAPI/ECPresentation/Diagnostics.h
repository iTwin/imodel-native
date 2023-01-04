/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/ECPresentationErrors.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_LOGGING

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class DiagnosticsCategory
    {
    Default,
    Serialization,
    Tasks,
    Connections,

    Performance,

    Rules,
    RulesetVariables,
    ECExpressions,

    Hierarchies,
    HierarchiesCache,

    Content,

    Update,
    HierarchiesUpdate,
    ContentUpdate,
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct Diagnostics
{
    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct Options
    {
    private:
        Nullable<uint64_t> m_minimumDuration;
        NativeLogging::SEVERITY m_devSeverity;
        NativeLogging::SEVERITY m_editorSeverity;
    public:
        Options() : m_devSeverity(NativeLogging::LOG_ERROR), m_editorSeverity(NativeLogging::LOG_ERROR) {}
        void SetMinimumDuration(Nullable<uint64_t> value) {m_minimumDuration = value;}
        Nullable<uint64_t> GetMinimumDuration() const {return m_minimumDuration;}
        void SetDevSeverity(NativeLogging::SEVERITY value) {m_devSeverity = value;}
        NativeLogging::SEVERITY GetDevSeverity() const {return m_devSeverity;}
        void SetEditorSeverity(NativeLogging::SEVERITY value) {m_editorSeverity = value;}
        NativeLogging::SEVERITY GetEditorSeverity() const {return m_editorSeverity;}
    };

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct ILogItem
    {
    protected:
        virtual rapidjson::Document _BuildJson(rapidjson::Document::AllocatorType*) const = 0;
    public:
        virtual ~ILogItem() {}
        rapidjson::Document BuildJson(rapidjson::Document::AllocatorType* allocator = nullptr) const {return _BuildJson(allocator);}
    };

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct Message : ILogItem
    {
    private:
        DiagnosticsCategory m_category;
        Utf8CP m_devSeverityStr;
        Utf8CP m_editorSeverityStr;
        uint64_t m_timestamp;
        Utf8String m_message;
    protected:
        rapidjson::Document _BuildJson(rapidjson::Document::AllocatorType* allocator) const override;
    public:
        Message(DiagnosticsCategory, NativeLogging::SEVERITY const* devSeverity, NativeLogging::SEVERITY const* editorSeverity, Utf8String message);
    };

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct Scope : std::enable_shared_from_this<Scope>, ILogItem
    {
        struct Holder
        {
        private:
            std::shared_ptr<Scope> m_scope;
        public:
            Holder() {}
            Holder(std::shared_ptr<Scope> scope) : m_scope(scope) {m_scope->Attach();}
            Holder(Holder const& other) = delete;
            Holder(Holder&& other) : m_scope(other.m_scope) {other.m_scope = nullptr;}
            ~Holder() {if (m_scope) {m_scope->Detach();}}
            operator bool() const {return m_scope != nullptr;}
            std::shared_ptr<Scope> operator*() const {return m_scope;}
            std::shared_ptr<Scope> operator=(std::shared_ptr<Scope> scope)
                {
                if (m_scope)
                    m_scope->Detach();
                m_scope = scope;
                if (m_scope)
                    m_scope->Attach();
                return m_scope;
                }
            Holder& operator=(Holder const& other) = delete;
            Holder& operator=(Holder&& other)
                {
                m_scope = other.m_scope;
                other.m_scope = nullptr;
                return *this;
                }
        };

    private:
        mutable BeMutex m_mutex;
        mutable bool m_isFinalized;
        std::weak_ptr<Scope> m_parentScope;
        std::list<std::shared_ptr<ILogItem>> m_logItems;
        std::shared_ptr<Options> m_options;
        Utf8String m_name;
        uint64_t m_start;
        uint64_t m_end;
        uint64_t m_threadId;
        bset<Utf8CP> m_capturedAttributes;
        bmap<Utf8CP, std::shared_ptr<bvector<Utf8String>>> m_arrayAttributes;

    private:
        Options const& GetOptions() const;
        uint64_t GetThreadId() const {return m_threadId;}
        void AddMessage(DiagnosticsCategory, NativeLogging::SEVERITY const*, NativeLogging::SEVERITY const*, Utf8String);
        void AddItem(std::shared_ptr<ILogItem>);

    protected:
        rapidjson::Document _BuildJson(rapidjson::Document::AllocatorType*) const override;

    public:
        Scope(std::shared_ptr<Scope> parentScope, Utf8String name, std::shared_ptr<Options> options);

    public:
        ECPRESENTATION_EXPORT static Holder ResetAndCreate(Utf8String name, Options options);
        ECPRESENTATION_EXPORT static Holder Create(Utf8String name, Options options);
        ECPRESENTATION_EXPORT static Holder Create(Utf8String name);

        ECPRESENTATION_EXPORT void Attach();
        ECPRESENTATION_EXPORT void Detach();
        Holder Hold() {return Holder(shared_from_this());}

        std::weak_ptr<Scope> GetParentScope() const {return m_parentScope;}
        uint64_t GetElapsedTime() const {return (m_end ? m_end : BeTimeUtilities::GetCurrentTimeAsUnixMillis()) - m_start;}

        ECPRESENTATION_EXPORT bool IsEnabled(DiagnosticsCategory, NativeLogging::SEVERITY devSeverity, NativeLogging::SEVERITY editorSeverity) const;
        ECPRESENTATION_EXPORT void Log(DiagnosticsCategory, NativeLogging::SEVERITY devSeverity, NativeLogging::SEVERITY editorSeverity, Utf8String msg);
        ECPRESENTATION_EXPORT void DevLog(DiagnosticsCategory, NativeLogging::SEVERITY, Utf8String msg);
        ECPRESENTATION_EXPORT void EditorLog(DiagnosticsCategory, NativeLogging::SEVERITY, Utf8String msg);
        ECPRESENTATION_EXPORT void SetCapturedAttributes(bvector<Utf8CP> const& attributes);
        ECPRESENTATION_EXPORT void AddValueToArrayAttribute(Utf8CP name, Utf8String value, bool unique);
    };

private:
    static void SetCurrentScope(std::shared_ptr<Scope>);
    static Scope* GetCurrentScopeRaw();

public:
    ECPRESENTATION_EXPORT static std::weak_ptr<Scope> GetCurrentScope();
    ECPRESENTATION_EXPORT static void Log(DiagnosticsCategory, NativeLogging::SEVERITY devSeverity, NativeLogging::SEVERITY editorSeverity, Utf8String msg);
    ECPRESENTATION_EXPORT static void DevLog(DiagnosticsCategory, NativeLogging::SEVERITY, Utf8String msg);
    ECPRESENTATION_EXPORT static void EditorLog(DiagnosticsCategory, NativeLogging::SEVERITY, Utf8String msg);
    ECPRESENTATION_EXPORT static void SetCapturedAttributes(bvector<Utf8CP> const& attributes);
    ECPRESENTATION_EXPORT static void AddValueToArrayAttribute(Utf8CP name, Utf8String value, bool unique);
};

#define DIAGNOSTICS_LOG(category, devSeverity, editorSeverity, msg)     Diagnostics::Log(category, devSeverity, editorSeverity, msg);
#define DIAGNOSTICS_DEV_LOG(category, severity, msg)                    Diagnostics::DevLog(category, severity, msg);
#define DIAGNOSTICS_EDITOR_LOG(category, severity, msg)                 Diagnostics::EditorLog(category, severity, msg);
#define DIAGNOSTICS_ASSERT_SOFT(category, condition, failureMsg)        { if (!(condition)) { BeAssert(false); Diagnostics::DevLog(category, LOG_ERROR, failureMsg); } }
#define DIAGNOSTICS_HANDLE_FAILURE(category, msg)                       { Diagnostics::DevLog(category, LOG_ERROR, msg); throw InternalError(msg); }

#define DIAGNOSTICS_SCOPE_ATTRIBUTE_Rules "rules"

END_BENTLEY_ECPRESENTATION_NAMESPACE
