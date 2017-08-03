/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelJs/iModelJsServicesTier.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <iModelJs/iModelJs.h>
#include <websocketpp/server.hpp>
#include <websocketpp/config/core.hpp>
#include <libuv/uv.h>
#include <atomic>
#include <functional>
#include <map>

/** @namespace BentleyApi::iModelJs::ServicesTier Contains types used by the services tier of the iModel.js application framework. */
#define BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE  BEGIN_BENTLEY_IMODELJS_NAMESPACE namespace ServicesTier {
#define END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE    } END_BENTLEY_IMODELJS_NAMESPACE

#define IMODELJS_SERVICES_TIER_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

#define IMODELJS_SERVICES_TIER_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

IMODELJS_SERVICES_TIER_TYPEDEFS (Extension)
IMODELJS_SERVICES_TIER_TYPEDEFS (Environment)
IMODELJS_SERVICES_TIER_TYPEDEFS (Host)
IMODELJS_SERVICES_TIER_TYPEDEFS (UvHost)
IMODELJS_SERVICES_TIER_TYPEDEFS (NodeHost)
IMODELJS_SERVICES_TIER_TYPEDEFS (Utilities)

IMODELJS_SERVICES_TIER_REF_COUNTED_PTR (Extension)

BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Extension : public RefCountedBase
    {
    friend struct Host;
    friend struct Environment;

public:
    typedef std::function<ExtensionPtr()> InstallCallback_T;

private:
    Js::RuntimeP m_runtime;

protected:
    IMODELJS_EXPORT virtual Utf8CP SupplyName() const = 0;
    IMODELJS_EXPORT virtual Js::Value ExportJsModule (Js::ScopeR scope) = 0;

public:
    IMODELJS_EXPORT static void Install (InstallCallback_T const& callback);

    IMODELJS_EXPORT Js::RuntimeR GetRuntime() const { return *m_runtime; }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Environment
    {
    friend struct Host;
    friend struct Extension;

private:
    static EnvironmentP s_instance;

    std::map<Utf8String, ExtensionPtr> m_extensions;

    Environment();
    ~Environment();

    void Install (ExtensionPtr extension);
    void Shutdown();
    Js::Value DeliverExtension (Js::ScopeR scope, Utf8StringCR identifier);

    static void InstallCoreExtensions();

public:
    IMODELJS_EXPORT static EnvironmentR GetInstance();
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Host
    {
    friend struct Extension;

public:
    DEFINE_POINTER_SUFFIX_TYPEDEFS (Config)

    typedef std::function<void()> EventLoopCallback_T;

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct Config
        {
        std::atomic<bool> enableJsDebugger;
        std::atomic<bool> waitForJsDebugger;
        std::atomic<uint16_t> jsDebuggerPort;

        Config() : enableJsDebugger  (true),
                   waitForJsDebugger (false),
                   jsDebuggerPort    (9229) { ; }
        };

private:
    static std::atomic<HostP> s_instance;
    
    std::atomic<uv_loop_t*> m_eventLoop;
    std::atomic<intptr_t> m_eventLoopThreadId;
    Js::RuntimeP m_jsRuntime;
    std::atomic<bool> m_ready;
    bool m_stopped;
    uv_idle_t m_idler;
    EnvironmentP m_environment;
    Js::Reference m_notifyIdle;
    Js::Reference m_notifyShutdown;
    Js::Reference m_notifyReady;

    static void IdleHandler (uv_idle_t* handle);
    static void EventLoopCallbackAsyncHandler (uv_async_t* handle);

    static void EventLoopAsyncCloseHandler (uv_handle_t* handle);
    
    static void DispatchExtensionCallback (Extension::InstallCallback_T const& callback);

    static Utf8CP InitScript();

    void HandleReady();
    void HandleIdle();
    
    void StartIdler();
    void StopIdler();
    void InitializeEnvironment();
    void TerminateEnvironment();
    void EmptyExtensionsQueue();
    void PerformInstall (Extension::InstallCallback_T const& callback);
    void SetupJsRuntime();
    void TeardownJsRuntime();

protected:
    IMODELJS_EXPORT Host();
    IMODELJS_EXPORT virtual ~Host();

    IMODELJS_EXPORT Utf8String GetSystemArgv() const;
    IMODELJS_EXPORT Utf8String GetSystemCwd() const;

    IMODELJS_EXPORT virtual uv_loop_t* SupplyEventLoop() = 0;
    IMODELJS_EXPORT virtual Js::RuntimeR SupplyJsRuntime() = 0;
    IMODELJS_EXPORT virtual Js::Function SupplyJsRequireHandler (Js::ScopeR scope, Js::ObjectCR initParams);
    IMODELJS_EXPORT virtual void SupplyJsInfoValues (Js::ScopeR scope, Js::ObjectR info);

    IMODELJS_EXPORT virtual void OnReady() { ; }
    IMODELJS_EXPORT virtual void OnIdle() { ; }
    IMODELJS_EXPORT virtual void OnStop() { ; }

    IMODELJS_EXPORT void NotifyStarting();
    IMODELJS_EXPORT void NotifyStop();

public:
    IMODELJS_EXPORT static bool Exists();
    IMODELJS_EXPORT static HostR GetInstance();
    IMODELJS_EXPORT static ConfigR GetConfig();

    IMODELJS_EXPORT uv_loop_t* GetEventLoop() const;
    IMODELJS_EXPORT Js::RuntimeR GetJsRuntime() const;

    IMODELJS_EXPORT bool IsReady() const;
    IMODELJS_EXPORT bool IsStopped() const;
    IMODELJS_EXPORT bool IsEventLoopThread() const;
    IMODELJS_EXPORT bool PostToEventLoop (EventLoopCallback_T const& callback);
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct UvHost : public Host
    {
private:
    uv_thread_t m_thread;
    Js::RuntimeP m_jsRuntime;

    static void EventLoopThreadEntry (void* arg);
    static Utf8CP RequireScript();

    void EventLoopThreadMain();

protected:
    IMODELJS_EXPORT uv_loop_t* SupplyEventLoop() override;
    IMODELJS_EXPORT Js::RuntimeR SupplyJsRuntime() override;
    IMODELJS_EXPORT Js::Function SupplyJsRequireHandler (Js::ScopeR scope, Js::ObjectCR initParams) override;
    IMODELJS_EXPORT void SupplyJsInfoValues (Js::ScopeR scope, Js::ObjectR info) override;

public:
    IMODELJS_EXPORT UvHost();
    IMODELJS_EXPORT ~UvHost() override;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct NodeHost : public Host
    {
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Utilities : public Extension
    {
public:
    DEFINE_POINTER_SUFFIX_TYPEDEFS (JsPrototypes)

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct JsPrototypes
        {
        Js::Reference uv_Handle;
        Js::Reference uv_Status;
        Js::Reference uv_io_Stream;
        Js::Reference uv_tcp_BindResult;
        Js::Reference uv_tcp_ConnectResult;
        Js::Reference uv_tcp_Handle;
        Js::Reference uv_tcp_Server;
        Js::Reference websocketpp_Base;
        Js::Reference websocketpp_ClientConnection;
        Js::Reference websocketpp_ServerEndpoint;
        };

private:
    JsPrototypes m_prototypes;

    static Utf8CP InitScript();

    Js::Function EvaluateInitScript (Js::ScopeR scope);
    Js::Object CreateInitParams (Js::ScopeR scope);
    void FindPrototypes (Js::ObjectCR exports);

    Js::Callback uv_Handle_close (Js::ScopeR scope);
    Js::Callback uv_io_shutdown (Js::ScopeR scope);
    Js::Callback uv_io_Stream_isReadable (Js::ScopeR scope);
    Js::Callback uv_io_Stream_isWritable (Js::ScopeR scope);
    Js::Callback uv_io_Stream_read (Js::ScopeR scope);
    Js::Callback uv_io_Stream_write (Js::ScopeR scope);
    Js::Callback uv_tcp_bind (Js::ScopeR scope);
    Js::Callback uv_tcp_connect (Js::ScopeR scope);
    Js::Callback uv_tcp_Handle_setNoDelay (Js::ScopeR scope);
    Js::Callback uv_tcp_Handle_setKeepAlive (Js::ScopeR scope);
    Js::Callback uv_tcp_Server_setSimultaneousAccepts (Js::ScopeR scope);
    Js::Callback uv_tcp_Server_listen (Js::ScopeR scope);
    Js::Callback uv_tcp_Server_accept (Js::ScopeR scope);
    Js::Callback websocketpp_Base_Dispose (Js::ScopeR scope);
    Js::Callback websocketpp_ClientConnection_process (Js::ScopeR scope);
    Js::Callback websocketpp_ClientConnection_send (Js::ScopeR scope);
    Js::Callback websocketpp_ServerEndpoint_constructor (Js::ScopeR scope);
    Js::Callback websocketpp_ServerEndpoint_createConnection (Js::ScopeR scope);
    Js::Callback uv_fs_open (Js::ScopeR scope);
    Js::Callback uv_fs_stat (Js::ScopeR scope);
    Js::Callback uv_fs_read (Js::ScopeR scope);
    Js::Callback uv_fs_close (Js::ScopeR scope);

protected:
    IMODELJS_EXPORT Utf8CP SupplyName() const override { return "@bentley/imodeljs-services-tier-utilities"; }
    IMODELJS_EXPORT Js::Value ExportJsModule (Js::ScopeR scope) override;

public:
    IMODELJS_EXPORT JsPrototypesCR GetPrototypes() const { return m_prototypes; }
    };

END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

//__PUBLISH_SECTION_END__
