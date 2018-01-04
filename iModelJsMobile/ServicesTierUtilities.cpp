/*--------------------------------------------------------------------------------------+
|
|     $Source: ServicesTierUtilities.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ServicesTierUtilities.h"

BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Object JsApi::Base::CreateStatus (Napi::Env& env, int uvResult) const
    {
    return uv_Status::Create (env, uvResult, m_owner.GetPrototypes().uv_Status.Value());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void JsApi::uv_Handle::CloseHandler (uv_handle_t* handle)
    {
    auto instance = reinterpret_cast<uv_HandleP>(handle->data);
    delete instance;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
JsApi::uv_io_Stream::~uv_io_Stream()
    {
    for (auto& entry : m_buffers)
        entry.second.Reset();
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void JsApi::uv_io_Stream::SetHandlers (Napi::Object allocator, Napi::Function readCallback)
    {
    m_allocator.Reset(allocator);
    m_readCallback.Reset(readCallback);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void JsApi::uv_io_Stream::StoreBuffer (Napi::ArrayBuffer buffer)
    {
    auto identifier = buffer.Data();
    BeAssert (m_buffers.find (identifier) == m_buffers.end());

    m_buffers[identifier] = Napi::ObjectReference::New(buffer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::ArrayBuffer JsApi::uv_io_Stream::GetBuffer (void* identifier)
    {
    auto it = m_buffers.find (identifier);
    BeAssert (it != m_buffers.end());

    return it->second.Value().As<Napi::ArrayBuffer>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void JsApi::uv_io_Stream::ReleaseBuffer (void* identifier)
    {
    auto it = m_buffers.find (identifier);
    if (it != m_buffers.end())
        {
        it->second.Reset();
        m_buffers.erase (it);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void JsApi::uv_io_Stream::AllocHandler (uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf)
    {
    BeAssert (handle->data != nullptr);
    
    buf->base = nullptr;
    buf->len = 0;

    auto instance = reinterpret_cast<uv_io_StreamP>(handle->data);
    BeAssert (!instance->GetAllocator().IsEmpty());

    if (Host::GetInstance().IsStopped())
        {
        instance->GetUvHandle().ClearAndClose();
        return;
        }

    auto& env = instance->GetOwner().Env();

    Napi::HandleScope scope (env);

    auto object = instance->GetAllocator();
    if (!object.Has ("allocate"))
        return;

    auto callback = object.Get("allocate").As<Napi::Function>();
    if (!callback.IsFunction())
        return;

    if (suggestedSize > Js::MAX_SAFE_JS_INTEGER)
        suggestedSize = Js::MAX_SAFE_JS_INTEGER;

    auto result = callback ({object, Napi::Number::New(env, static_cast<double>(suggestedSize))});
    if (!result.IsArrayBuffer())
        return;

    auto buffer = result.As<Napi::ArrayBuffer>();
    instance->StoreBuffer (buffer);

    buf->base = reinterpret_cast<char*>(buffer.Data());
    buf->len = static_cast<ULONG>(buffer.ByteLength());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void JsApi::uv_io_Stream::ReadHandler (uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
    {
    BeAssert (stream->data != nullptr);

    auto instance = reinterpret_cast<uv_io_StreamP>(stream->data);
    BeAssert (!instance->GetAllocator().IsEmpty() && !instance->GetReadCallback().IsEmpty());

    if (Host::GetInstance().IsStopped())
        {
        uv_read_stop (stream);
        instance->GetUvHandle().ClearAndClose();
        return;
        }

    auto& env = instance->GetOwner().Env();

    Napi::HandleScope scope (instance->GetOwner().Env());

    auto readCallback = instance->GetReadCallback().As<Napi::Function>();
    auto object = instance->GetObject();

    auto status = instance->CreateStatus (env, static_cast<int>(nread));

    auto validBuffer = (nread >= 0);

    Napi::Value bufferValue = env.Null();
    if (validBuffer)
        bufferValue = instance->GetBuffer (buf->base);
    
    auto result = readCallback({object, status, bufferValue, Napi::Number::New(env, static_cast<double>(nread))});

    bool stop = true;
    if (result.IsBoolean() && result.As<Napi::Boolean>().Value())
        stop = false;

    if (stop)
        uv_read_stop (stream);

    if (validBuffer)
        {
        auto buffer = bufferValue.As<Napi::ArrayBuffer>();
        auto release = true;

        auto allocator = instance->GetAllocator();
        if (allocator.Has ("recycle"))
            {
            auto recycleCallback = object.Get("recycle").As<Napi::Function>();
            if (recycleCallback.IsFunction())
                {
                auto result = recycleCallback ({allocator, buffer});
                if (result.IsBoolean() && result.As<Napi::Boolean>().Value())
                    release = false;
                }
            }

        if (release)
            instance->ReleaseBuffer (buf->base);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void JsApi::uv_io_shutdown_Promise::Handler (uv_shutdown_t* req, int status)
    {
    BeAssert (req->data != nullptr);

    auto& instance = *reinterpret_cast<uv_io_shutdown_PromiseP>(req->data);

    if (Host::GetInstance().IsStopped())
        {
        UvIoStream (req->handle).ClearAndClose();
        delete &instance;
        return;
        }

    auto& env = instance.GetOwner().Env();

    Napi::HandleScope scope (env);

    auto result = instance.CreateStatus (env, status);

    if (status >= 0)
        instance.m_promise.Resolve(result);
    else
        instance.m_promise.Reject(result);

    delete &instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
JsApi::uv_io_shutdown_Promise::uv_io_shutdown_Promise (UtilitiesCR owner, UvIoStreamShutdownRequest&& request)
    : Base      (owner),
      m_request (std::move (request)),
      m_promise (Napi::Promise::Deferred::New(owner.Env()))
    {
    m_request.GetPointer()->data = this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Object JsApi::uv_io_shutdown_Promise::Create (UtilitiesCR owner, UvIoStreamShutdownRequest&& request)
    {
    new uv_io_shutdown_Promise (owner, std::move (request)); //deleted by Handler
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
JsApi::uv_io_Stream_write_Promise::uv_io_Stream_write_Promise (UtilitiesCR owner, UvIoStreamWriteRequest&& request)
    : Base      (owner),
      m_request (std::move (request)),
      m_promise (Napi::Promise::Deferred::New(owner.Env()))
    {
    m_request.GetPointer()->data = this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Object JsApi::uv_io_Stream_write_Promise::Create (UtilitiesCR owner, UvIoStreamWriteRequest&& request)
    {
    new uv_io_Stream_write_Promise (owner, std::move (request)); //deleted by Handler
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void JsApi::uv_io_Stream_write_Promise::Handler (uv_write_t* req, int status)
    {
    BeAssert (req->data != nullptr);

    auto& instance = *reinterpret_cast<uv_io_Stream_write_PromiseP>(req->data);
    
    if (Host::GetInstance().IsStopped())
        {
        UvIoStream (req->handle).ClearAndClose();
        delete &instance;
        return;
        }

    auto& env = instance.GetOwner().Env();

    Napi::HandleScope scope (env);

    auto result = instance.CreateStatus (env, status);

    if (status >= 0)
        instance.m_promise.Resolve (result);
    else
        instance.m_promise.Reject (result);

    delete &instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
JsApi::uv_tcp_connect_Promise::uv_tcp_connect_Promise (UtilitiesCR owner, UvTcpHandle&& handle, UvTcpConnectRequest&& request)
    : Base      (owner),
      m_handle  (std::move (handle)),
      m_request (std::move (request)),
      m_promise (Napi::Promise::Deferred::New(owner.Env()))
    {
    m_request.GetPointer()->data = this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Object JsApi::uv_tcp_connect_Promise::Create (UtilitiesCR owner, UvTcpHandle&& handle, UvTcpConnectRequest&& request)
    {
    new uv_tcp_connect_Promise (owner, std::move (handle), std::move (request)); //deleted by Handler
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void JsApi::uv_tcp_connect_Promise::Handler (uv_connect_t* req, int status)
    {
    BeAssert (req->data != nullptr);

    auto& instance = *reinterpret_cast<uv_tcp_connect_PromiseP>(req->data);

    if (Host::GetInstance().IsStopped())
        {
        instance.GetUvTcpHandle().ClearAndClose();
        delete &instance;
        return;
        }
    
    auto& env = instance.GetOwner().Env();
    
    Napi::HandleScope scope (env);

    if (status >= 0)
        {
        auto connection = uv_tcp_Handle::Create (instance.GetOwner(), std::move (instance.m_handle));
        auto result = uv_tcp_ConnectResult::Create (instance.GetOwner(), status, connection);
        instance.m_promise.Resolve (result);
        }
    else
        {
        instance.GetUvTcpHandle().Close();
        auto result = uv_tcp_ConnectResult::Create (instance.GetOwner(), status, env.Null());
        instance.m_promise.Reject (result);
        }

    delete &instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Object JsApi::uv_Status::Create (Napi::Env& env, int uvResult, Napi::Value prototype)
    {
    auto object = Napi::Object::New(env);

    BeAssert(false && "WIP_NAPI_PROTOTYPE");    
    #ifdef WIP_NAPI_PROTOTYPE
    auto setPrototypeResult = object.SetPrototype (prototype);
    BeAssert (setPrototypeResult);
#endif

    auto code = JsApi::uv_StatusCode::Create (uvResult);

    object.Set ("code", Napi::Number::New(env, code));
    
    return object;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Object JsApi::uv_tcp_BindResult::Create (UtilitiesCR owner, int uvResult, Napi::Value server)
    {
    auto object = uv_Status::Create (owner.Env(), uvResult, owner.GetPrototypes().uv_tcp_BindResult.Value());

    object.Set("server", server);

    return object;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Object JsApi::uv_tcp_ConnectResult::Create (UtilitiesCR owner, int uvResult, Napi::Value connection)
    {
    auto object = uv_Status::Create (owner.Env(), uvResult, owner.GetPrototypes().uv_tcp_ConnectResult.Value());

    object.Set ("connection", connection);

    return object;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
JsApi::uv_tcp_Handle::uv_tcp_Handle (UtilitiesCR owner, UvTcpHandle&& handle)
    : uv_io_Stream (owner),
      m_handle     (std::move (handle))
    {
    m_handle.GetPointer()->data = this;
    #ifdef WIP_EXTERNAL
    SetObject(Napi::External<uv_tcp_Handle>::New(owner.Env(), this));
    #endif
    BeAssert(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Object JsApi::uv_tcp_Handle::Create (UtilitiesCR owner, UvTcpHandle&& handle)
    {
    auto instance = new uv_tcp_Handle (owner, std::move (handle)); //deleted by uv_Handle::CloseHandler

    auto object = instance->GetObject();
    BeAssert(false && "WIP_NAPI_PROTOTYPE");    
    #ifdef WIP_NAPI_PROTOTYPE
    object.SetPrototype (owner.GetPrototypes().uv_tcp_Handle.Value());
    #endif

    return object;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Object JsApi::uv_tcp_Server::Create (UtilitiesCR owner, UvTcpHandle&& handle)
    {
    auto instance = new uv_tcp_Server (owner, std::move (handle)); //deleted by uv_Handle::CloseHandler

    auto object = instance->GetObject();
    BeAssert(false && "WIP_NAPI_PROTOTYPE");    
    #ifdef WIP_NAPI_PROTOTYPE
    object.SetPrototype (owner.GetPrototypes().uv_tcp_Server.Value());
    #endif

    return object;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void JsApi::uv_tcp_Server::ListenHandler (uv_stream_t* server, int status)
    {
    auto& instance = *reinterpret_cast<uv_tcp_ServerP>(server->data);
    BeAssert (instance.GetUvIoStream().GetPointer() == server);
    BeAssert (!instance.GetListenCallback().IsEmpty());

    if (Host::GetInstance().IsStopped())
        {
        UvTcpHandle (reinterpret_cast<uv_tcp_t*>(server)).ClearAndClose();
        return;
        }
    
    auto& env = instance.GetOwner().Env();
    Napi::HandleScope scope (env);
    auto callback = instance.GetListenCallback().As<Napi::Function>();
    auto object = instance.GetObject();

    if (status >= 0)
        {
        UvTcpHandle handle (status);
        if (status >= 0)
            {
            auto connection = JsApi::uv_tcp_Handle::Create (instance.GetOwner(), std::move (handle));
            callback ({object, connection, instance.CreateStatus (env, status)});
            return;
            }
        }

    BeAssert (status < 0);
    callback ({object, env.Null(), instance.CreateStatus (env, status)});
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void JsApi::uv_tcp_Server::SetListenCallback (Napi::Function value)
    {
    m_listenCallback.Reset(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
JsApi::websocketpp_ServerEndpoint::websocketpp_ServerEndpoint (UtilitiesCR owner, Napi::CallbackInfo const& info)
    : ObjectBase (owner)
    {
    m_server.clear_access_channels (websocketpp::log::alevel::all);
    m_server.clear_error_channels (websocketpp::log::elevel::all);

#ifdef WIP_EXTERNAL
    SetObject(Napi::External<JsApi::websocketpp_ServerEndpoint>::New(info.Env(), this));
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Object JsApi::websocketpp_ServerEndpoint::Create (UtilitiesCR owner, Napi::CallbackInfo const& info)
    {
    auto instance = new websocketpp_ServerEndpoint (owner, info); //deleted by websocketpp_Base_Dispose

    auto object = instance->GetObject();
    auto setPrototypeResult = object.SetPrototype (owner.GetPrototypes().websocketpp_ServerEndpoint.Get());
    BeAssert (setPrototypeResult);

    return object;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
JsApi::websocketpp_ClientConnection::websocketpp_ClientConnection (UtilitiesCR owner, websocketpp_connection_ptr_t const& connection)
    : ObjectBase   (owner),
      m_connection (connection),
      m_relay      (*this),
      m_output     (&m_relay)
    {
    GetObject().Assign (owner.Env(), Napi::External<JsApi::websocketpp_ClientConnection>::New(owner.Env(), this));
    
    Forwarder forwarder (*this);
    m_connection->set_open_handler (forwarder);
    m_connection->set_fail_handler (forwarder);
    m_connection->set_message_handler (forwarder);

    m_connection->register_ostream (&m_output);
    m_connection->start();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Object JsApi::websocketpp_ClientConnection::Create (UtilitiesCR owner, websocketpp_connection_ptr_t const& connection)
    {
    auto instance = new websocketpp_ClientConnection (owner, info, connection); //deleted by websocketpp_Base_Dispose

    auto object = instance->GetObject().Get().AsObject();
    auto setPrototypeResult = object.SetPrototype (owner.GetPrototypes().websocketpp_ClientConnection.Get());
    BeAssert (setPrototypeResult);

    auto setHandlerProperty = object.Set ("handler", info.CreateNull());
    BeAssert (setHandlerProperty);

    return object;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
std::streamsize JsApi::websocketpp_ClientConnection::Relay::xsputn (const char_type* s, std::streamsize count)
    {
    Napi::HandleScope scope (m_connection.GetOwner().Env());

    auto object = m_connection.GetObject().Get().AsObject();
    if (object.Has ("handler"))
        {
        auto handler = object.Get ("handler").AsNoTypeCheck<Napi::Object>();
        if (handler.IsObject())
            {
            auto callback = handler.Get ("transport").AsNoTypeCheck<Napi::Function>();
            if (callback.IsFunction())
                {
                auto buffer = scope.CreateArrayBuffer ((void*)s, count);
                callback (object, buffer);
                }
            }
        }

    return count;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void JsApi::websocketpp_ClientConnection::Forwarder::operator() (websocketpp::connection_hdl handle)
    {
    Napi::HandleScope scope (m_connection.GetOwner().Env());

    auto object = m_connection.GetObject().Get().AsObject();
    if (!object.Has ("handler"))
        return;

    auto handler = object.Get ("handler").AsNoTypeCheck<Napi::Object>();
    if (!handler.IsObject())
        return;

    auto& connection = m_connection.GetConnection();
    if (connection.get_state() == websocketpp::session::state::open)
        {
        if (handler.Has ("open"))
            {
            auto callback = handler.Get ("open").AsNoTypeCheck<Napi::Function>();
            if (callback.IsFunction())
                callback (object);
            }
        }
    else
        {
        if (handler.Has ("fail"))
            {
            auto callback = handler.Get ("fail").AsNoTypeCheck<Napi::Function>();
            if (callback.IsFunction())
                callback (object);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void JsApi::websocketpp_ClientConnection::Forwarder::operator() (websocketpp::connection_hdl handle, websocketpp_ServerEndpoint::websocketpp_server_t::message_ptr message)
    {
    Napi::HandleScope scope (m_connection.GetOwner().Env());

    auto object = m_connection.GetObject().Get().AsObject();
    if (!object.Has ("handler"))
        return;

    auto handler = object.Get ("handler").AsNoTypeCheck<Napi::Object>();
    if (!handler.IsObject() || !handler.Has ("message"))
        return;

    auto callback = handler.Get ("message").AsNoTypeCheck<Napi::Function>();
    if (!callback.IsFunction())
        return;

    auto opcode = message->get_opcode();
    auto& payload = message->get_raw_payload();
    
    auto payloadBuffer = scope.CreateArrayBuffer (&payload [0], payload.size());
    callback (object, payloadBuffer, info.CreateNumber (opcode));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
UvNetAddressDescriptor::UvNetAddressDescriptor (Napi::String address, Napi::Number port, Napi::Number protocol, int& status)
    {
    if (protocol.IsEqual<uint32_t>(JsApi::uv_net_IP::V6))
        {
        m_protocol = IP::V6;
        status = uv_ip6_addr (address.GetValue().c_str(), port.CastValue<int>(), &m_addr6);
        }
    else
        {
        m_protocol = IP::V4;
        status = uv_ip4_addr (address.GetValue().c_str(), port.CastValue<int>(), &m_addr4);
        }

    BeAssert (status >= 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
const sockaddr* UvNetAddressDescriptor::GetPointer() const
    {
    if (m_protocol == IP::V6)
        return reinterpret_cast<const sockaddr*>(&m_addr6);
    else
        return reinterpret_cast<const sockaddr*>(&m_addr4);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
UvHandle::UvHandle (UvHandle&& other)
    : m_handle (other.m_handle)
    {
    other.m_handle = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
UvHandle::~UvHandle()
    {
    if (m_handle != nullptr)
        {
        BeAssert (m_handle->data != nullptr);
        free (m_handle);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void UvHandle::CloseHandler (uv_handle_t* handle)
    {
    BeAssert (handle->data == nullptr);

    free (handle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void UvHandle::Close()
    {
    if (m_handle == nullptr)
        return;

    BeAssert (m_handle->data == nullptr);

    uv_close (m_handle, &CloseHandler);
    m_handle = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void UvHandle::ClearAndClose()
    {
    if (m_handle == nullptr)
        return;

    m_handle->data = nullptr;
    Close();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
UvBuffer::UvBuffer (char* base, unsigned int length)
    {
    m_buffer = uv_buf_init (base, length);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
UvBuffer::UvBuffer (Napi::ArrayBuffer buffer)
    : UvBuffer (reinterpret_cast<char*>(buffer.GetValue()), static_cast<unsigned int>(buffer.GetLength()))
    {
    ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
uv_stream_t* UvIoStream::GetPointer() const
    {
    auto handle = UvHandle::GetPointer();
    BeAssert (handle->type == UV_STREAM || handle->type == UV_TCP);

    return reinterpret_cast<uv_stream_t*>(handle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
UvTcpHandle::UvTcpHandle (int& status)
    : UvIoStream (reinterpret_cast<uv_stream_t*>(malloc (sizeof (uv_tcp_t))))
    {
    auto handle = reinterpret_cast<uv_tcp_t*>(UvHandle::GetPointer());
    
    status = uv_tcp_init (Host::GetInstance().GetEventLoop(), handle);
    BeAssert (status >= 0);
    
    handle->data = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
uv_tcp_t* UvTcpHandle::GetPointer() const
    {
    auto handle = UvHandle::GetPointer();
    BeAssert (handle->type == UV_TCP);

    return reinterpret_cast<uv_tcp_t*>(handle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
UvRequest::UvRequest (UvRequest&& other)
    : m_request (other.m_request)
    {
    other.m_request = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
UvRequest::~UvRequest()
    {
    if (m_request != nullptr)
        free (m_request);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
UvIoStreamShutdownRequest::UvIoStreamShutdownRequest (UvIoStreamCR handle, uv_shutdown_cb callback, int& status)
    : UvRequest (reinterpret_cast<uv_req_t*>(malloc (sizeof (uv_shutdown_t))))
    {
    auto request = reinterpret_cast<uv_shutdown_t*>(UvRequest::GetPointer());

    status = uv_shutdown (request, handle.GetPointer(), callback);
    BeAssert (status >= 0);

    request->data = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
uv_shutdown_t* UvIoStreamShutdownRequest::GetPointer() const
    {
    auto request = UvRequest::GetPointer();
    BeAssert (request->type = UV_SHUTDOWN);

    return reinterpret_cast<uv_shutdown_t*>(request);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
UvIoStreamWriteRequest::UvIoStreamWriteRequest (UvIoStreamCR handle, UvBufferCR buffer, uv_write_cb callback, int& status)
    : UvRequest (reinterpret_cast<uv_req_t*>(malloc (sizeof (uv_write_t))))
    {
    auto request = reinterpret_cast<uv_write_t*>(UvRequest::GetPointer());

    status = uv_write (request, handle.GetPointer(), buffer.GetPointer(), 1, callback);

    request->data = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
uv_write_t* UvIoStreamWriteRequest::GetPointer() const
    {
    auto request = UvRequest::GetPointer();
    BeAssert (request->type = UV_WRITE);

    return reinterpret_cast<uv_write_t*>(request);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
UvTcpConnectRequest::UvTcpConnectRequest (UvNetAddressDescriptorCR address, UvTcpHandleCR handle, uv_connect_cb callback, int& status)
    : UvRequest (reinterpret_cast<uv_req_t*>(malloc (sizeof (uv_connect_t))))
    {
    auto request = reinterpret_cast<uv_connect_t*>(UvRequest::GetPointer());

    status = ::uv_tcp_connect (request, handle.GetPointer(), address.GetPointer(), callback);
    BeAssert (status >= 0);

    request->data = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
uv_connect_t* UvTcpConnectRequest::GetPointer() const
    {
    auto request = UvRequest::GetPointer();
    BeAssert (request->type = UV_CONNECT);

    return reinterpret_cast<uv_connect_t*>(request);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::EvaluateInitScript (Js::RuntimeR runtime)
    {
    auto evaluation = runtime.EvaluateScript (InitScript());
    BeAssert (evaluation.status == napi_ok);

    return evaluation.value.As<Napi::Function>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_Handle_close (Napi::Env& env)
    {
    return scope.CreateCallback ([](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_EXTERNAL_THIS;

        auto instance = info.CastThis<JsApi::uv_Handle>();
        uv_close (instance->GetUvHandle().GetPointer(), &JsApi::uv_Handle::CloseHandler);

        return JS_CALLBACK_UNDEFINED;
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_io_shutdown (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (1);

        auto stream = JS_CALLBACK_EXTERNAL (0);

        int status;
        auto streamInstance = stream.Cast<JsApi::uv_io_Stream>();

        UvIoStreamShutdownRequest request (streamInstance->GetUvIoStream(), &JsApi::uv_io_shutdown_Promise::Handler, status);
        if (status >= 0)
            return JsApi::uv_io_shutdown_Promise::Create (streamInstance->GetOwner(), info, std::move (request));

        BeAssert (status < 0);
        auto result = streamInstance->CreateStatus (info, status);

        return JsApi::Promise::CreateAndReject (info, result);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_io_Stream_isReadable (Napi::Env& env)
    {
    return scope.CreateCallback ([](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_EXTERNAL_THIS;

        auto instance = info.CastThis<JsApi::uv_io_Stream>();
        auto result = uv_is_readable (instance->GetUvIoStream().GetPointer());

        return JS_CALLBACK_BOOLEAN (result == 1);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_io_Stream_isWritable (Napi::Env& env)
    {
    return scope.CreateCallback ([](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_EXTERNAL_THIS;

        auto instance = info.CastThis<JsApi::uv_io_Stream>();
        auto result = uv_is_writable (instance->GetUvIoStream().GetPointer());

        return JS_CALLBACK_BOOLEAN (result == 1);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_io_Stream_read (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (2);
        JS_CALLBACK_REQUIRE_EXTERNAL_THIS;

        auto allocator    = JS_CALLBACK_GET_OBJECT   (0);
        auto readCallback = JS_CALLBACK_GET_FUNCTION (1);
        
        auto instance = info.CastThis<JsApi::uv_io_Stream>();
        instance->SetHandlers (allocator, readCallback);

        auto result = uv_read_start (instance->GetUvIoStream().GetPointer(), &JsApi::uv_io_Stream::AllocHandler, &JsApi::uv_io_Stream::ReadHandler);

        return instance->CreateStatus (info, result);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_io_Stream_write (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (1);
        JS_CALLBACK_REQUIRE_EXTERNAL_THIS;

        auto data = JS_CALLBACK_GET_ARRAY_BUFFER (0);

        int status;
        auto instance = info.CastThis<JsApi::uv_io_Stream>();

        UvBuffer buffer (data);
        UvIoStreamWriteRequest request (instance->GetUvIoStream(), buffer, &JsApi::uv_io_Stream_write_Promise::Handler, status);
        if (status >= 0)
            return JsApi::uv_io_Stream_write_Promise::Create (instance->GetOwner(), info, std::move (request));

        BeAssert (status < 0);
        auto result = instance->CreateStatus (info, status);

        return JsApi::Promise::CreateAndReject (info, result);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_tcp_bind (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (3);

        auto address  = JS_CALLBACK_GET_STRING (0);
        auto port     = JS_CALLBACK_GET_NUMBER (1);
        auto protocol = JS_CALLBACK_GET_NUMBER (2);

        int status;

        UvNetAddressDescriptor descriptor (address, port, protocol, status);
        if (status >= 0)
            {
            UvTcpHandle handle (status);
            if (status >= 0)
                {
                status = ::uv_tcp_bind (handle.GetPointer(), descriptor.GetPointer(), 0);
                if (status >= 0)
                    {
                    auto server = JsApi::uv_tcp_Server::Create (*this, info, std::move (handle));
                    auto result = JsApi::uv_tcp_BindResult::Create (*this, info, status, server);

                    return result;
                    }
                else
                    {
                    handle.Close();
                    }
                }
            }

        BeAssert (status <= 0);
        auto result = JsApi::uv_tcp_BindResult::Create (*this, info, status, JS_CALLBACK_NULL);

        return result;
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_tcp_connect (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (3);

        auto address  = JS_CALLBACK_GET_STRING (0);
        auto port     = JS_CALLBACK_GET_NUMBER (1);
        auto protocol = JS_CALLBACK_GET_NUMBER (2);

        int status;

        UvNetAddressDescriptor descriptor (address, port, protocol, status);
        if (status >= 0)
            {
            UvTcpHandle handle (status);
            if (status >= 0)
                {
                UvTcpConnectRequest request (descriptor, handle, &JsApi::uv_tcp_connect_Promise::Handler, status);
                if (status >= 0)
                    return JsApi::uv_tcp_connect_Promise::Create (*this, info, std::move (handle), std::move (request));
                else
                    handle.Close();
                }
            }

        BeAssert (status <= 0);
        auto result = JsApi::uv_tcp_ConnectResult::Create (*this, info, status, JS_CALLBACK_NULL);

        return JsApi::Promise::CreateAndReject (info, result);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_tcp_Handle_setNoDelay (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (1);
        JS_CALLBACK_REQUIRE_EXTERNAL_THIS;

        auto enable = JS_CALLBACK_GET_BOOLEAN (0);

        auto instance = info.CastThis<JsApi::uv_tcp_Handle>();
        auto result = uv_tcp_nodelay (instance->GetUvTcpHandle().GetPointer(), enable.GetValue());

        return instance->CreateStatus (info, result);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_tcp_Handle_setKeepAlive (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (2);
        JS_CALLBACK_REQUIRE_EXTERNAL_THIS;

        auto enable = JS_CALLBACK_GET_BOOLEAN (0);
        auto delay  = JS_CALLBACK_GET_NUMBER  (1);

        auto instance = info.CastThis<JsApi::uv_tcp_Handle>();
        auto result = uv_tcp_keepalive (instance->GetUvTcpHandle().GetPointer(), enable.GetValue(), static_cast<unsigned int>(delay.GetValue()));

        return instance->CreateStatus (info, result);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_tcp_Server_setSimultaneousAccepts (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (1);
        JS_CALLBACK_REQUIRE_EXTERNAL_THIS;

        auto enable = JS_CALLBACK_GET_BOOLEAN (0);

        auto instance = info.CastThis<JsApi::uv_tcp_Handle>();
        auto result = uv_tcp_simultaneous_accepts (instance->GetUvTcpHandle().GetPointer(), enable.GetValue());

        return instance->CreateStatus (info, result);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_tcp_Server_listen (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (2);
        JS_CALLBACK_REQUIRE_EXTERNAL_THIS;

        auto backlog  = JS_CALLBACK_GET_NUMBER   (0);
        auto callback = JS_CALLBACK_GET_FUNCTION (1);

        auto instance = info.CastThis<JsApi::uv_tcp_Server>();
        instance->SetListenCallback (callback);

        auto result = uv_listen (instance->GetUvIoStream().GetPointer(), static_cast<int>(backlog.GetValue()), &JsApi::uv_tcp_Server::ListenHandler);

        return instance->CreateStatus (info, result);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_tcp_Server_accept (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (1);
        JS_CALLBACK_REQUIRE_EXTERNAL_THIS;

        auto connection = JS_CALLBACK_GET_EXTERNAL (0);

        auto instance = info.CastThis<JsApi::uv_tcp_Server>();
        auto connectionInstance = connection.Cast<JsApi::uv_tcp_Handle>();
        auto result = uv_accept (instance->GetUvIoStream().GetPointer(), connectionInstance->GetUvIoStream().GetPointer());

        return instance->CreateStatus (info, result);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::websocketpp_Base_Dispose (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_EXTERNAL_THIS;

        auto instance = info.CastThis<JsApi::ObjectBase>();
        delete &instance;

        return JS_CALLBACK_UNDEFINED;
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::websocketpp_ClientConnection_process (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (3);
        JS_CALLBACK_REQUIRE_EXTERNAL_THIS;

        auto input = JS_CALLBACK_GET_ARRAY_BUFFER (0);
        auto offset = JS_CALLBACK_GET_NUMBER (1);
        auto length = JS_CALLBACK_GET_NUMBER (2);

        auto instance = info.CastThis<JsApi::websocketpp_ClientConnection>();

        auto result = instance->GetConnection().read_all (reinterpret_cast<const char*>(input.GetValue()) + static_cast<uint32_t>(offset.GetValue()),
                                                          static_cast<uint32_t>(length.GetValue()));

        return JS_CALLBACK_BOOLEAN (result == static_cast<uint32_t>(length.GetValue()));
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::websocketpp_ClientConnection_send (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (2);
        JS_CALLBACK_REQUIRE_EXTERNAL_THIS;

        auto message = JS_CALLBACK_GET_ARRAY_BUFFER (0);
        auto code = JS_CALLBACK_GET_NUMBER (1);

        auto instance = info.CastThis<JsApi::websocketpp_ClientConnection>();

        auto result = instance->GetConnection().send (message.GetValue(), message.GetLength(), static_cast<websocketpp::frame::opcode::value>(static_cast<uint32_t>(code.GetValue())));

        return JS_CALLBACK_BOOLEAN (result.value() == 0);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::websocketpp_ServerEndpoint_constructor (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        return JsApi::websocketpp_ServerEndpoint::Create (*this, info);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::websocketpp_ServerEndpoint_createConnection (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_EXTERNAL_THIS;

        auto instance = info.CastThis<JsApi::websocketpp_ServerEndpoint>();

        auto connection = instance->GetServer().get_connection();
        if (connection == nullptr)
            return JS_CALLBACK_NULL;
        else
            return JsApi::websocketpp_ClientConnection::Create (*this, info, connection);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_fs_open (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (3);

        auto path  = JS_CALLBACK_GET_STRING (0);
        auto flags = JS_CALLBACK_GET_NUMBER (1);
        auto mode  = JS_CALLBACK_GET_NUMBER (2);

        uv_fs_t req;
        ::uv_fs_open (Host::GetInstance().GetEventLoop(), &req, path.GetValue().c_str(), static_cast<int>(flags.GetValue()), static_cast<int>(mode.GetValue()), nullptr);
        uv_fs_req_cleanup (&req);

        return JS_CALLBACK_NUMBER (static_cast<double>(req.result));
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_fs_stat (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (1);

        auto file = JS_CALLBACK_GET_NUMBER (0);

        uv_fs_t req;
        auto result = ::uv_fs_fstat (Host::GetInstance().GetEventLoop(), &req, static_cast<uv_file>(file.GetValue()), nullptr);
        
        if (result < 0)
            {
            uv_fs_req_cleanup (&req);

            return JS_CALLBACK_NULL;
            }
        else
            {
            auto object = JS_CALLBACK_OBJECT;
            object.Set ("size", JS_CALLBACK_NUMBER (static_cast<double>(req.statbuf.st_size)));
            object.Set ("isFile", JS_CALLBACK_BOOLEAN ((req.statbuf.st_mode & S_IFREG) == S_IFREG));
            object.Set ("isDirectory", JS_CALLBACK_BOOLEAN ((req.statbuf.st_mode & S_IFDIR) == S_IFDIR));

            uv_fs_req_cleanup (&req);

            return object;
            }
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_fs_read (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (2);

        auto file = JS_CALLBACK_GET_NUMBER (0);
        auto destination = JS_CALLBACK_GET_ARRAY_BUFFER (1);

        uv_fs_t req;
        auto buf = uv_buf_init (static_cast<char*>(destination.GetValue()), static_cast<unsigned int>(destination.GetLength()));
        auto result = ::uv_fs_read (Host::GetInstance().GetEventLoop(), &req, static_cast<uv_file>(file.GetValue()), &buf, 1, 0, nullptr);
        uv_fs_req_cleanup (&req);

        return JS_CALLBACK_BOOLEAN (result >= 0);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Function Utilities::uv_fs_close (Napi::Env& env)
    {
    return scope.CreateCallback ([this](Napi::CallbackInfo const& info) -> Napi::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (1);

        auto file = JS_CALLBACK_GET_NUMBER (0);

        uv_fs_t req;
        auto result = ::uv_fs_close (Host::GetInstance().GetEventLoop(), &req, static_cast<uv_file>(file.GetValue()), nullptr);
        uv_fs_req_cleanup (&req);

        return JS_CALLBACK_BOOLEAN (result >= 0);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Object Utilities::CreateInitParams(Napi::Env& env)
    {
    auto params = Napi::Object::New(env);
    params.Set ("uv_Handle_close",                             uv_Handle_close (scope));
    params.Set ("uv_io_shutdown",                              uv_io_shutdown (scope));
    params.Set ("uv_io_Stream_isReadable",                     uv_io_Stream_isReadable (scope));
    params.Set ("uv_io_Stream_isWritable",                     uv_io_Stream_isWritable (scope));
    params.Set ("uv_io_Stream_read",                           uv_io_Stream_read (scope));
    params.Set ("uv_io_Stream_write",                          uv_io_Stream_write (scope));
    params.Set ("uv_tcp_bind",                                 uv_tcp_bind (scope));
    params.Set ("uv_tcp_connect",                              uv_tcp_connect (scope));
    params.Set ("uv_tcp_Handle_setNoDelay",                    uv_tcp_Handle_setNoDelay (scope));
    params.Set ("uv_tcp_Handle_setKeepAlive",                  uv_tcp_Handle_setKeepAlive (scope));
    params.Set ("uv_tcp_Server_setSimultaneousAccepts",        uv_tcp_Server_setSimultaneousAccepts (scope));
    params.Set ("uv_tcp_Server_listen",                        uv_tcp_Server_listen (scope));
    params.Set ("uv_tcp_Server_accept",                        uv_tcp_Server_accept (scope));
    params.Set ("websocketpp_Base_Dispose",                    websocketpp_Base_Dispose (scope));
    params.Set ("websocketpp_ClientConnection_process",        websocketpp_ClientConnection_process (scope));
    params.Set ("websocketpp_ClientConnection_send",           websocketpp_ClientConnection_send (scope));
    params.Set ("websocketpp_ServerEndpoint_constructor",      websocketpp_ServerEndpoint_constructor (scope));
    params.Set ("websocketpp_ServerEndpoint_createConnection", websocketpp_ServerEndpoint_createConnection (scope));
    params.Set ("uv_fs_open",                                  uv_fs_open (scope));
    params.Set ("uv_fs_stat",                                  uv_fs_stat (scope));
    params.Set ("uv_fs_read",                                  uv_fs_read (scope));
    params.Set ("uv_fs_close",                                 uv_fs_close (scope));

    static_assert (O_RDONLY     <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_WRONLY     <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_RDWR       <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_APPEND     <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_CREAT      <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_TRUNC      <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_EXCL       <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_TEXT       <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_BINARY     <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_RAW        <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_TEMPORARY  <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_NOINHERIT  <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_SEQUENTIAL <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (O_RANDOM     <= Js::MAX_SAFE_JS_INTEGER, "Js");

    static_assert (S_IFMT   <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (S_IFDIR  <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (S_IFCHR  <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (S_IFREG  <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (S_IREAD  <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (S_IWRITE <= Js::MAX_SAFE_JS_INTEGER, "Js");
    static_assert (S_IEXEC  <= Js::MAX_SAFE_JS_INTEGER, "Js");

    params.Set ("uv_fs_O_RDONLY",     scope.CreateNumber (O_RDONLY));
    params.Set ("uv_fs_O_WRONLY",     scope.CreateNumber (O_WRONLY));
    params.Set ("uv_fs_O_RDWR",       scope.CreateNumber (O_RDWR));
    params.Set ("uv_fs_O_APPEND",     scope.CreateNumber (O_APPEND));
    params.Set ("uv_fs_O_CREAT",      scope.CreateNumber (O_CREAT));
    params.Set ("uv_fs_O_TRUNC",      scope.CreateNumber (O_TRUNC));
    params.Set ("uv_fs_O_EXCL",       scope.CreateNumber (O_EXCL));
    params.Set ("uv_fs_O_TEXT",       scope.CreateNumber (O_TEXT));
    params.Set ("uv_fs_O_BINARY",     scope.CreateNumber (O_BINARY));
    params.Set ("uv_fs_O_RAW",        scope.CreateNumber (O_RAW));
    params.Set ("uv_fs_O_TEMPORARY",  scope.CreateNumber (O_TEMPORARY));
    params.Set ("uv_fs_O_NOINHERIT",  scope.CreateNumber (O_NOINHERIT));
    params.Set ("uv_fs_O_SEQUENTIAL", info.CreateNumber (O_SEQUENTIAL));
    params.Set ("uv_fs_O_RANDOM",     scope.CreateNumber (O_RANDOM));

    params.Set ("uv_fs_S_IFMT",   scope.CreateNumber (S_IFMT));
    params.Set ("uv_fs_S_IFDIR",  scope.CreateNumber (S_IFDIR));
    params.Set ("uv_fs_S_IFCHR",  scope.CreateNumber (S_IFCHR));
    params.Set ("uv_fs_S_IFREG",  scope.CreateNumber (S_IFREG));
    params.Set ("uv_fs_S_IREAD",  scope.CreateNumber (S_IREAD));
    params.Set ("uv_fs_S_IWRITE", info.CreateNumber (S_IWRITE));
    params.Set ("uv_fs_S_IEXEC",  scope.CreateNumber (S_IEXEC));

    return params;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Utilities::FindPrototypes (Napi::Object exports)
    {
    auto& env = Env();
    
    auto exports_uv = exports.GetAsObject ("uv");
    m_prototypes.uv_Handle.Assign (env, exports_uv.GetAsObject ("Handle").Get ("prototype"));
    m_prototypes.uv_Status.Assign (env, exports_uv.GetAsObject ("Status").Get ("prototype"));
    
    auto exports_uv_io = exports_uv.GetAsObject ("io");
    m_prototypes.uv_io_Stream.Assign            (env, exports_uv_io.GetAsObject ("Stream").Get ("prototype"));
    
    auto exports_uv_tcp = exports_uv.GetAsObject ("tcp");
    m_prototypes.uv_tcp_BindResult.Assign    (env, exports_uv_tcp.GetAsObject ("BindResult").Get ("prototype"));
    m_prototypes.uv_tcp_ConnectResult.Assign (env, exports_uv_tcp.GetAsObject ("ConnectResult").Get ("prototype"));
    m_prototypes.uv_tcp_Handle.Assign        (env, exports_uv_tcp.GetAsObject ("Handle").Get ("prototype"));
    m_prototypes.uv_tcp_Server.Assign        (env, exports_uv_tcp.GetAsObject ("Server").Get ("prototype"));

    auto exports_websocketpp = exports.GetAsObject ("websocketpp");
    m_prototypes.websocketpp_Base.Assign (env, exports_websocketpp.GetAsObject ("Base").Get ("prototype"));
    m_prototypes.websocketpp_ClientConnection.Assign (env, exports_websocketpp.GetAsObject ("ClientConnection").Get ("prototype"));
    m_prototypes.websocketpp_ServerEndpoint.Assign   (env, exports_websocketpp.GetAsObject ("ServerEndpoint").Get ("prototype"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Napi::Value Utilities::ExportJsModule(Js::RuntimeR runtime)
    {
    auto initScript = EvaluateInitScript();
    auto initParams = CreateInitParams(runtime.Env());

    auto exports = initScript (runtime.Env().GetGlobal(), initParams).AsObject();
    FindPrototypes (exports);

    return exports;
    }

END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE
