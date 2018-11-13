/*--------------------------------------------------------------------------------------+
|
|     $Source: BeJavaScript/V8InspectorClientProxy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4100)
    #pragma warning(disable:4244)
    #pragma warning(disable:4251)
#endif

#include <google_v8/v8-inspector.h>

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

#include "JsDebugServer.h"

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
// @bsiclass                                              Marius.Balcaitis    08/2017
//=======================================================================================
struct V8InspectorClientProxy : RefCounted<NonCopyableClass>
    {
private:
    //=======================================================================================
    // @bsiclass                                              Marius.Balcaitis    08/2017
    //=======================================================================================
    struct InspectorClient : public v8_inspector::V8InspectorClient
        {
    private:
        V8InspectorClientProxy& m_proxy;
        bool m_shouldQuitMessageLoop;

    public:
        InspectorClient(V8InspectorClientProxy& proxy) : m_proxy(proxy), m_shouldQuitMessageLoop (false) {};

        void runMessageLoopOnPause(int contextGroupId) override;
        void quitMessageLoopOnPause() override;
        void runIfWaitingForDebugger(int contextGroupId) override;
        };

    //=======================================================================================
    // @bsiclass                                              Marius.Balcaitis    08/2017
    //=======================================================================================
    struct Channel : public v8_inspector::V8Inspector::Channel
        {
    private:
        V8InspectorClientProxy& m_proxy;

        void send(int callId, std::unique_ptr<v8_inspector::StringBuffer>& message);

    public:
        Channel(V8InspectorClientProxy& proxy) : m_proxy(proxy) {};

        void sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message) override;
        void sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message) override;
        void flushProtocolNotifications() override;
        };

    static bmap<v8::Isolate*, RefCountedPtr<V8InspectorClientProxy>> s_proxies;
    static bool s_isDebuggingEnabled;

    v8::Isolate* m_isolate;
    const char* m_listeningPort;
    JsDebugServer::IConnectionPtr m_connection;

    std::unique_ptr<InspectorClient> m_inspectorClient;
    std::unique_ptr<Channel> m_channel;
    std::unique_ptr<v8_inspector::V8Inspector> m_inspector;
    std::unique_ptr<v8_inspector::V8InspectorSession> m_session;

    void DispatchProtocolMessage(const std::string& msg);
    void ResetInspector();

    bool IsEnabled();
    void Enable();
    void Disable();

protected:
    V8InspectorClientProxy(v8::Isolate* isolate);
    ~V8InspectorClientProxy();

public:
    static V8InspectorClientProxy& GetInstance(v8::Isolate* isolate);
    static void DestroyInstance(v8::Isolate* isolate);

    static void EnableDebugging();
    static void DisableDebugging();

    void AttachContext(v8::Local<v8::Context> context, const char* name);
    void DetachContext(v8::Local<v8::Context> context);

    void SetListeningPort(const char* port);
    };

END_BENTLEY_NAMESPACE
