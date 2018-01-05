/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelJs/iModelJsServicesTier.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    IMODELJS_EXPORT virtual Napi::Value ExportJsModule (Js::RuntimeR) = 0;

public:
    IMODELJS_EXPORT static void Install (InstallCallback_T const& callback);

    IMODELJS_EXPORT Js::RuntimeR GetRuntime() const { return *m_runtime; }
    IMODELJS_EXPORT Napi::Env& Env() const { return GetRuntime().Env(); }
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
    Napi::Value DeliverExtension (Js::RuntimeR, Utf8StringCR identifier);

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

        Config() : enableJsDebugger  (false),
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
    Napi::ObjectReference m_notifyIdle;
    Napi::ObjectReference m_notifyShutdown;
    Napi::ObjectReference m_notifyReady;

    static void IdleHandler (uv_idle_t* handle);
    static void EventLoopCallbackAsyncHandler (uv_async_t* handle);

    static void EventLoopAsyncCloseHandler (uv_handle_t* handle);
    
    static void DispatchExtensionCallback (Extension::InstallCallback_T const& callback);

    static Utf8CP InitScript();
    void DefineNodeWorkAlikes();

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
    IMODELJS_EXPORT virtual Napi::Function SupplyJsRequireHandler (Napi::Env& env, Napi::Object initParams);
    IMODELJS_EXPORT virtual void SupplyJsInfoValues (Napi::Env& env, Napi::Object info);

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
    Napi::Env& Env() {return GetJsRuntime().Env();}

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
    IMODELJS_EXPORT Napi::Function SupplyJsRequireHandler (Napi::Env& env, Napi::Object initParams) override;
    IMODELJS_EXPORT void SupplyJsInfoValues (Napi::Env& env, Napi::Object info) override;

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
        Napi::ObjectReference uv_Handle;
        Napi::ObjectReference uv_Status;
        Napi::ObjectReference uv_io_Stream;
        Napi::ObjectReference uv_tcp_BindResult;
        Napi::ObjectReference uv_tcp_ConnectResult;
        Napi::ObjectReference uv_tcp_Handle;
        Napi::ObjectReference uv_tcp_Server;
        Napi::ObjectReference websocketpp_Base;
        Napi::ObjectReference websocketpp_ClientConnection;
        Napi::ObjectReference websocketpp_ServerEndpoint;
        };

private:
    JsPrototypes m_prototypes;

#ifdef COMMENT_OUT
    static Utf8CP InitScript();
    Napi::Function EvaluateInitScript(Js::RuntimeR);

    Napi::Function uv_Handle_close (Napi::Env& env);
    Napi::Function uv_io_shutdown (Napi::Env& env);
    Napi::Function uv_io_Stream_isReadable (Napi::Env& env);
    Napi::Function uv_io_Stream_isWritable (Napi::Env& env);
    Napi::Function uv_io_Stream_read (Napi::Env& env);
    Napi::Function uv_io_Stream_write (Napi::Env& env);
    Napi::Function uv_tcp_bind (Napi::Env& env);
    Napi::Function uv_tcp_connect (Napi::Env& env);
    Napi::Function uv_tcp_Handle_setNoDelay (Napi::Env& env);
    Napi::Function uv_tcp_Handle_setKeepAlive (Napi::Env& env);
    Napi::Function uv_tcp_Server_setSimultaneousAccepts (Napi::Env& env);
    Napi::Function uv_tcp_Server_listen (Napi::Env& env);
    Napi::Function uv_tcp_Server_accept (Napi::Env& env);
    Napi::Function websocketpp_Base_Dispose (Napi::Env& env);
    Napi::Function websocketpp_ClientConnection_process (Napi::Env& env);
    Napi::Function websocketpp_ClientConnection_send (Napi::Env& env);
    Napi::Function websocketpp_ServerEndpoint_constructor (Napi::Env& env);
    Napi::Function websocketpp_ServerEndpoint_createConnection (Napi::Env& env);
#endif

    static Utf8CP SimpleInitScript();
    Napi::Function EvaluateSimpleInitScript(Js::RuntimeR);
    Napi::Object CreateInitParams (Napi::Env& env);
    void FindPrototypes (Napi::Object exports);
    
    Napi::Function uv_fs_open (Napi::Env& env);
    Napi::Function uv_fs_stat (Napi::Env& env);
    Napi::Function uv_fs_read (Napi::Env& env);
    Napi::Function uv_fs_close (Napi::Env& env);

protected:
    IMODELJS_EXPORT Utf8CP SupplyName() const override { return "@bentley/imodeljs-services-tier-utilities"; }
    IMODELJS_EXPORT Napi::Value ExportJsModule (Js::RuntimeR) override;

public:
    IMODELJS_EXPORT JsPrototypesCR GetPrototypes() const { return m_prototypes; }
    };

//=======================================================================================
// @bsiclass                                                    Sam.Wilson   01/18
//=======================================================================================
struct NodeWorkAlike
    {
    struct Globals 
        {
        static void Install(Js::RuntimeR);
        };

    struct Extension_path : Extension 
        {
        Utf8CP SupplyName() const override {return "path";}
        Napi::Value ExportJsModule (Js::RuntimeR) override;
        };
    };

END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

//__PUBLISH_SECTION_END__
