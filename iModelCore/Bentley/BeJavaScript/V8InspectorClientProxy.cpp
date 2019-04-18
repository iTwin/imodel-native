/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "V8InspectorClientProxy.h"
#include "V8InspectorManager.h"
#include <codecvt>

BEGIN_BENTLEY_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::InspectorClient::runMessageLoopOnPause(int contextGroupId)
    {
    if (m_proxy.m_connection.IsNull() || !m_proxy.m_connection->IsConnected())
        return;

    m_shouldQuitMessageLoop = false;
    while (!m_shouldQuitMessageLoop)
        {
        if (0 >= m_proxy.m_connection->Recv())
            {
            m_proxy.ResetInspector();
            return;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::InspectorClient::quitMessageLoopOnPause()
    {
    m_shouldQuitMessageLoop = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::InspectorClient::runIfWaitingForDebugger(int contextGroupId)
    {
    // This is executed when debugger is attached. We could pause JS execution or do some other usefull stuff here if needed...
    // m_proxy.dispatchProtocolMessage(R"({"id":1,"method":"Debugger.pause"})");
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                               Marius.Balcaitis    08/2017
//
// std::codecvt<char16_t, char, std::mbstate_t> does not have public destructor so we
// have to wrap it to adapt for std::wstring_convert
//--------------+------------------------------------------------------------------------
template<class Facet>
struct DeletableFacet : Facet
    {
    static std::locale::id id;

    template<class ...Args>
    DeletableFacet(Args&& ...args) : Facet(std::forward<Args>(args)...) {}
    ~DeletableFacet() {}
    };

template<class Facet>
std::locale::id DeletableFacet<Facet>::id;

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::Channel::send(int callId, std::unique_ptr<v8_inspector::StringBuffer>& message)
    {
    if (m_proxy.m_connection.IsNull() || !m_proxy.m_connection->IsConnected())
        return;

    v8_inspector::StringView stringView = message->string();

    int nBytesSent;
    if (stringView.is8Bit())
        {
        nBytesSent = m_proxy.m_connection->Send(reinterpret_cast<const void*>(stringView.characters8()), stringView.length());
        }
    else
        {
        std::wstring_convert<DeletableFacet<std::codecvt<char16_t, char, std::mbstate_t>>, char16_t> converter;

        std::string result = converter.to_bytes(reinterpret_cast<const char16_t*>(
            stringView.characters16()),
            reinterpret_cast<const char16_t*>(stringView.characters16() + stringView.length()));

        nBytesSent = m_proxy.m_connection->Send(reinterpret_cast<const void*>(result.c_str()), result.length());
        }

    if (nBytesSent < 0)
        {
        m_proxy.ResetInspector();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::Channel::sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message)
    {
    send(callId, message);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::Channel::sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message)
    {
    send(-1, message);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::Channel::flushProtocolNotifications()
    {
    // Nothing to do here, no way to flush stuff sent over TCP socket except maybe waiting for acknowledgement.
    }

bmap<v8::Isolate*, RefCountedPtr<V8InspectorClientProxy>> V8InspectorClientProxy::s_proxies;
bool V8InspectorClientProxy::s_isDebuggingEnabled = false;

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
V8InspectorClientProxy::V8InspectorClientProxy(v8::Isolate* isolate)
    {
    m_isolate = isolate;
    m_listeningPort = nullptr;

    m_inspectorClient.reset(new InspectorClient(*this));
    m_inspector = v8_inspector::V8Inspector::create(isolate, m_inspectorClient.get());
    m_channel.reset(new Channel(*this));

    m_session = m_inspector->connect(1, m_channel.get(), v8_inspector::StringView());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
V8InspectorClientProxy::~V8InspectorClientProxy()
    {
    Disable();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
V8InspectorClientProxy& V8InspectorClientProxy::GetInstance(v8::Isolate* isolate)
    {
    BeAssert(isolate != nullptr);

    auto it = s_proxies.find(isolate);
    if (it == s_proxies.end())
        {
        V8InspectorClientProxy* proxy = new V8InspectorClientProxy(isolate);
        s_proxies.insert(bpair<v8::Isolate*, RefCountedPtr<V8InspectorClientProxy>>(isolate, proxy));
        return *proxy;
        }
    return *(it->second);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::DestroyInstance(v8::Isolate* isolate)
    {
    if (isolate == nullptr)
        return;

    auto it = s_proxies.find(isolate);
    if (it == s_proxies.end())
        {
        return;
        }

    it->second->Disable();
    s_proxies.erase(it);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::EnableDebugging()
    {
    if (s_isDebuggingEnabled)
        return;

    s_isDebuggingEnabled = true;
    for (auto it = s_proxies.begin(); it != s_proxies.end(); it++)
        {
        it->second->Enable();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::DisableDebugging()
    {
    if (!s_isDebuggingEnabled)
        return;

    s_isDebuggingEnabled = false;
    for (auto it = s_proxies.begin(); it != s_proxies.end(); it++)
        {
        it->second->Disable();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::AttachContext(v8::Local<v8::Context> context, const char* name)
    {
    m_inspector->contextCreated(v8_inspector::V8ContextInfo(
        context,
        1,
        v8_inspector::StringView(reinterpret_cast<const uint8_t *>(name), strlen(name))));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::DetachContext(v8::Local<v8::Context> context)
    {
    m_inspector->contextDestroyed(context);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::SetListeningPort(const char* port)
    {
    if (IsEnabled())
        Disable();

    m_listeningPort = port;

    if (s_isDebuggingEnabled)
        Enable();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::Enable()
    {
    if (IsEnabled())
        return;

    if (m_listeningPort == nullptr)
        return;

    auto messageHandler = [this](const std::string& payload)
        {
        this->DispatchProtocolMessage(payload);
        };

    m_connection = JsDebugServer::GetInstance().Listen(messageHandler, m_listeningPort);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::Disable()
    {
    if (!IsEnabled())
        return;

    if (m_connection->IsConnected())
        ResetInspector();

    m_connection->Close();
    m_connection = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
bool V8InspectorClientProxy::IsEnabled()
    {
    return !m_connection.IsNull();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::DispatchProtocolMessage(const std::string& msg)
    {
    v8_inspector::StringView msgView(reinterpret_cast<const uint8_t *>(msg.c_str()), msg.length());
    m_session->dispatchProtocolMessage(msgView);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorClientProxy::ResetInspector()
    {
    DispatchProtocolMessage(R"({"id":1,"method":"Profiler.disable"})");
    DispatchProtocolMessage(R"({"id":2,"method":"Runtime.disable"})");
    DispatchProtocolMessage(R"({"id":3,"method":"Debugger.disable"})");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorManager::AttachIsolate(v8::Isolate* isolate, const char* port)
    {
    auto& proxy = V8InspectorClientProxy::GetInstance(isolate);
    proxy.SetListeningPort(port);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorManager::DetachIsolate(v8::Isolate* isolate)
    {
    V8InspectorClientProxy::DestroyInstance(isolate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorManager::AttachContext(v8::Local<v8::Context> context, const char* name)
    {
    auto& proxy = V8InspectorClientProxy::GetInstance(context->GetIsolate());
    proxy.AttachContext(context, name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorManager::DetachContext(v8::Local<v8::Context> context)
    {
    auto& proxy = V8InspectorClientProxy::GetInstance(context->GetIsolate());
    proxy.DetachContext(context);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorManager::SetMainThreadDispatcher(void(*mainThreadDispatcher)(void(*callback)(void*arg), void*arg))
    {
    JsDebugServer::SetMainThreadDispatcher(mainThreadDispatcher);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorManager::EnableDebugging()
    {
    V8InspectorClientProxy::EnableDebugging();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Marius.Balcaitis    08/2017
//--------------+------------------------------------------------------------------------
void V8InspectorManager::DisableDebugging()
    {
    V8InspectorClientProxy::DisableDebugging();
    }

END_BENTLEY_NAMESPACE
